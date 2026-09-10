import Foundation

/// A single "MIDI learn" binding: incoming CC on a channel drives one
/// track's parameter.
struct CCMapping: Codable, Equatable, Identifiable {
    var id: UUID = UUID()
    var channel: Int        // 0...15
    var controller: Int     // 0...127
    var trackIndex: Int
    var parameter: ParameterID
}

enum ClockSource: String, Codable, CaseIterable, Identifiable {
    case internalClock
    case external

    var id: String { rawValue }
    var displayName: String { self == .internalClock ? "Internal" : "External MIDI Clock" }
}

/// Everything about how this project talks to the outside MIDI world.
/// Endpoints are matched by display name at connect time (CoreMIDI
/// endpoint refs aren't stable across app launches or device reconnects).
struct MIDISettings: Codable, Equatable {
    var inputEndpointName: String?
    var outputEndpointName: String?
    var clockSource: ClockSource = .internalClock
    var sendClock: Bool = false
    var ccMappings: [CCMapping] = []
    /// When true, an incoming Program Change message selects the pattern
    /// with that index within the current bank.
    var programChangeSelectsPattern: Bool = true

    mutating func addOrReplaceMapping(_ mapping: CCMapping) {
        if let i = ccMappings.firstIndex(where: {
            $0.channel == mapping.channel && $0.controller == mapping.controller
        }) {
            ccMappings[i] = mapping
        } else {
            ccMappings.append(mapping)
        }
    }
}
