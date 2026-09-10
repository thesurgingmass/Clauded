import AVFoundation
import CoreMIDI
import Foundation

struct MIDIEndpointInfo: Identifiable, Hashable {
    let id: MIDIEndpointRef
    let name: String
}

/// Thin CoreMIDI wrapper: enumerates endpoints, connects to one input and
/// one output by display name, parses incoming MIDI 1.0 bytes, and can
/// send note/clock/transport messages back out. Deliberately knows
/// nothing about `Project`/`Sequencer` — callers wire the closures below
/// to whatever they want done with the parsed messages, so this class
/// stays a reusable MIDI I/O layer.
final class MIDIManager: ObservableObject {
    @Published private(set) var availableInputs: [MIDIEndpointInfo] = []
    @Published private(set) var availableOutputs: [MIDIEndpointInfo] = []
    @Published private(set) var connectedInputName: String?
    @Published private(set) var connectedOutputName: String?
    @Published private(set) var isLearning = false

    // MARK: - Callbacks wired up by AppState

    var onNoteOn: ((_ channel: Int, _ note: Int, _ velocity: Int) -> Void)?
    var onNoteOff: ((_ channel: Int, _ note: Int) -> Void)?
    var onCC: ((_ channel: Int, _ controller: Int, _ value: Int) -> Void)?
    var onProgramChange: ((_ channel: Int, _ program: Int) -> Void)?
    var onClockTick: ((_ hostTime: UInt64) -> Void)?
    var onStart: (() -> Void)?
    var onStop: (() -> Void)?
    var onContinue: (() -> Void)?
    var onMappingLearned: ((CCMapping) -> Void)?

    private var client = MIDIClientRef()
    private var inputPort = MIDIPortRef()
    private var outputPort = MIDIPortRef()
    private var connectedSource: MIDIEndpointRef?
    private var connectedDestination: MIDIEndpointRef?
    private var learnTarget: (trackIndex: Int, parameter: ParameterID)?

    init() {
        setupClient()
        refreshEndpoints()
    }

    // MARK: - Setup

    private func setupClient() {
        MIDIClientCreateWithBlock("OctaClone" as CFString, &client) { [weak self] _ in
            self?.refreshEndpoints()
        }
        MIDIInputPortCreateWithBlock(client, "OctaClone Input" as CFString, &inputPort) { [weak self] packetListPtr, _ in
            self?.handle(packetList: packetListPtr)
        }
        MIDIOutputPortCreate(client, "OctaClone Output" as CFString, &outputPort)
    }

    func refreshEndpoints() {
        var inputs: [MIDIEndpointInfo] = []
        for i in 0..<MIDIGetNumberOfSources() {
            let src = MIDIGetSource(i)
            inputs.append(MIDIEndpointInfo(id: src, name: displayName(for: src)))
        }
        var outputs: [MIDIEndpointInfo] = []
        for i in 0..<MIDIGetNumberOfDestinations() {
            let dst = MIDIGetDestination(i)
            outputs.append(MIDIEndpointInfo(id: dst, name: displayName(for: dst)))
        }
        DispatchQueue.main.async {
            self.availableInputs = inputs
            self.availableOutputs = outputs
        }
    }

    private func displayName(for endpoint: MIDIEndpointRef) -> String {
        var unmanagedName: Unmanaged<CFString>?
        let status = MIDIObjectGetStringProperty(endpoint, kMIDIPropertyDisplayName, &unmanagedName)
        guard status == noErr, let cfName = unmanagedName?.takeRetainedValue() else { return "Unknown Device" }
        return cfName as String
    }

    // MARK: - Connections

    func connectInput(named name: String?) {
        if let connectedSource {
            MIDIPortDisconnectSource(inputPort, connectedSource)
        }
        connectedSource = nil
        connectedInputName = nil
        guard let name else { return }
        for i in 0..<MIDIGetNumberOfSources() {
            let src = MIDIGetSource(i)
            if displayName(for: src) == name {
                MIDIPortConnectSource(inputPort, src, nil)
                connectedSource = src
                connectedInputName = name
                break
            }
        }
    }

    func connectOutput(named name: String?) {
        connectedDestination = nil
        connectedOutputName = nil
        guard let name else { return }
        for i in 0..<MIDIGetNumberOfDestinations() {
            let dst = MIDIGetDestination(i)
            if displayName(for: dst) == name {
                connectedDestination = dst
                connectedOutputName = name
                break
            }
        }
    }

    // MARK: - MIDI learn

    func beginLearning(trackIndex: Int, parameter: ParameterID) {
        learnTarget = (trackIndex, parameter)
        isLearning = true
    }

    func cancelLearning() {
        learnTarget = nil
        isLearning = false
    }

    // MARK: - Receiving

    private func handle(packetList packetListPtr: UnsafePointer<MIDIPacketList>) {
        // MIDIPacketNext advances by (header size + packet.length), so it
        // must walk the *actual* CoreMIDI buffer, not a local copy of a
        // packet struct — taking the address through the mutable pointer's
        // `.pointee.packet` keeps us pointed into that real buffer.
        let mutableListPtr = UnsafeMutablePointer(mutating: packetListPtr)
        withUnsafeMutablePointer(to: &mutableListPtr.pointee.packet) { firstPacketPtr in
            var currentPtr = firstPacketPtr
            for _ in 0..<packetListPtr.pointee.numPackets {
                let length = Int(currentPtr.pointee.length)
                let timeStamp = currentPtr.pointee.timeStamp
                let bytes = withUnsafeBytes(of: currentPtr.pointee.data) { raw -> [UInt8] in
                    Array(raw.prefix(length))
                }
                process(bytes: bytes, timeStamp: timeStamp)
                currentPtr = MIDIPacketNext(currentPtr)
            }
        }
    }

    private func process(bytes: [UInt8], timeStamp: MIDITimeStamp) {
        var i = 0
        while i < bytes.count {
            let byte = bytes[i]
            guard byte >= 0x80 else { i += 1; continue } // desynced/data byte with no status; skip

            if byte >= 0xF8 {
                handleRealtime(status: byte, timeStamp: timeStamp)
                i += 1
                continue
            }

            let messageType = byte & 0xF0
            let channel = Int(byte & 0x0F)

            switch messageType {
            case 0x80, 0x90, 0xA0, 0xB0, 0xE0:
                guard i + 2 < bytes.count else { return }
                handleChannelMessage(status: messageType, channel: channel, data1: bytes[i + 1], data2: bytes[i + 2])
                i += 3
            case 0xC0, 0xD0:
                guard i + 1 < bytes.count else { return }
                handleChannelMessage(status: messageType, channel: channel, data1: bytes[i + 1], data2: 0)
                i += 2
            default:
                return // SysEx / system-common: not handled
            }
        }
    }

    private func handleRealtime(status: UInt8, timeStamp: MIDITimeStamp) {
        switch status {
        case 0xF8: onClockTick?(timeStamp)
        case 0xFA: onStart?()
        case 0xFB: onContinue?()
        case 0xFC: onStop?()
        default: break
        }
    }

    private func handleChannelMessage(status: UInt8, channel: Int, data1: UInt8, data2: UInt8) {
        switch status {
        case 0x90 where data2 > 0:
            onNoteOn?(channel, Int(data1), Int(data2))
        case 0x90, 0x80:
            onNoteOff?(channel, Int(data1))
        case 0xB0:
            if let target = learnTarget {
                let mapping = CCMapping(channel: channel, controller: Int(data1), trackIndex: target.trackIndex, parameter: target.parameter)
                learnTarget = nil
                DispatchQueue.main.async {
                    self.isLearning = false
                    self.onMappingLearned?(mapping)
                }
            } else {
                onCC?(channel, Int(data1), Int(data2))
            }
        case 0xC0:
            onProgramChange?(channel, Int(data1))
        default:
            break
        }
    }

    // MARK: - Sending

    private func sendPacket(bytes: [UInt8], atHostTime hostTime: UInt64) {
        guard let destination = connectedDestination else { return }
        var packetList = MIDIPacketList()
        withUnsafeMutablePointer(to: &packetList) { listPtr in
            let packetPtr = MIDIPacketListInit(listPtr)
            _ = MIDIPacketListAdd(listPtr, 1024, packetPtr, hostTime, bytes.count, bytes)
            MIDISend(outputPort, destination, listPtr)
        }
    }

    func sendClockTick(atHostTime hostTime: UInt64) {
        sendPacket(bytes: [0xF8], atHostTime: hostTime)
    }

    func sendStart() { sendPacket(bytes: [0xFA], atHostTime: 0) }
    func sendStop() { sendPacket(bytes: [0xFC], atHostTime: 0) }
    func sendContinue() { sendPacket(bytes: [0xFB], atHostTime: 0) }

    /// Sends note-on at `atHostTime` and automatically follows up with a
    /// note-off shortly after. The off is only wall-clock-scheduled (not
    /// sample-accurate) since it's a secondary "notify other gear" feature,
    /// not part of this app's own audio timing.
    func sendNoteOn(channel: Int, note: UInt8, velocity: UInt8, atHostTime hostTime: UInt64) {
        let statusByte = UInt8(0x90 | (channel & 0x0F))
        sendPacket(bytes: [statusByte, note & 0x7F, velocity & 0x7F], atHostTime: hostTime)
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.15) { [weak self] in
            self?.sendNoteOff(channel: channel, note: note)
        }
    }

    func sendNoteOff(channel: Int, note: UInt8) {
        let statusByte = UInt8(0x80 | (channel & 0x0F))
        sendPacket(bytes: [statusByte, note & 0x7F, 0], atHostTime: 0)
    }
}
