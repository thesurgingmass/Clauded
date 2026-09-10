import AVFoundation
import Darwin
import Foundation

/// Drives playback. Runs a look-ahead scheduler (the classic "many small
/// wake-ups, schedule audio slightly ahead of real time" pattern) when
/// using the internal clock, or is driven tick-by-tick by incoming MIDI
/// clock bytes when synced externally.
///
/// Threading note: this is a small hobby-scope project, not a
/// professionally hardened realtime audio app. The clock runs on its own
/// high-priority dispatch queue and reads `Project`/`Track` state (whose
/// properties are otherwise mutated from the main thread by the UI)
/// without additional locking. That's a benign race for simple value
/// reads/writes of Bool/Double/Int and is called out here rather than
/// hidden, but a from-scratch realtime-audio product should use
/// lock-free snapshots instead.
final class Sequencer: ObservableObject {
    static let ticksPerQuarterNote = 24
    /// One bar of 16 sixteenth-notes, in ticks. Pattern switches are
    /// quantized to this boundary regardless of any individual track's
    /// own (possibly polymetric) length.
    static let barLengthTicks = 16 * TrackScale.normal.ticksPerStep

    @Published private(set) var isPlaying = false
    @Published private(set) var currentStepPerTrack: [Int]
    @Published private(set) var pendingPatternIndex: Int?
    @Published private(set) var estimatedExternalBPM: Double?

    let project: Project
    private let audioEngine: AudioEngine
    weak var midiManager: MIDIManager?

    private var timer: DispatchSourceTimer?
    private let clockQueue = DispatchQueue(label: "com.octaclone.sequencer.clock", qos: .userInteractive)

    private var globalTick: Int = 0
    private var hostStartTime: UInt64 = 0
    private var nextTickOffsetSeconds: Double = 0
    private let scheduleAheadSeconds: Double = 0.15
    private let pollInterval: Double = 0.01

    init(project: Project, audioEngine: AudioEngine) {
        self.project = project
        self.audioEngine = audioEngine
        self.currentStepPerTrack = Array(repeating: 0, count: project.tracks.count)
    }

    private var secondsPerTick: Double {
        60.0 / max(1, project.bpm) / Double(Sequencer.ticksPerQuarterNote)
    }

    // MARK: - Transport

    func togglePlay() {
        isPlaying ? stop() : start()
    }

    /// Manual start. No-op while synced to an external clock, which starts
    /// playback only on receiving a MIDI Start/Continue message.
    func start() {
        guard !isPlaying, project.midi.clockSource == .internalClock else { return }
        beginPlaying()
        startInternalTimer()
        midiManager?.sendStart()
    }

    func stop() {
        guard isPlaying else { return }
        isPlaying = false
        timer?.cancel()
        timer = nil
        audioEngine.stopAll()
        if project.midi.clockSource == .internalClock {
            midiManager?.sendStop()
        }
    }

    private func beginPlaying() {
        isPlaying = true
        globalTick = 0
        hostStartTime = mach_absolute_time()
        nextTickOffsetSeconds = 0
    }

    private func startInternalTimer() {
        let t = DispatchSource.makeTimerSource(queue: clockQueue)
        t.schedule(deadline: .now(), repeating: pollInterval)
        t.setEventHandler { [weak self] in self?.pollInternal() }
        t.resume()
        timer = t
    }

    private func pollInternal() {
        let elapsed = AVAudioTime.seconds(forHostTime: mach_absolute_time() - hostStartTime)
        while nextTickOffsetSeconds < elapsed + scheduleAheadSeconds {
            let tickHostTime = hostStartTime &+ AVAudioTime.hostTime(forSeconds: nextTickOffsetSeconds)
            fireTick(globalTick, hostTime: tickHostTime)
            if project.midi.sendClock {
                midiManager?.sendClockTick(atHostTime: tickHostTime)
            }
            globalTick += 1
            nextTickOffsetSeconds += secondsPerTick
        }
    }

    // MARK: - External MIDI clock

    /// Called on every incoming 0xF8 clock byte when `clockSource == .external`.
    /// `hostTime` is the CoreMIDI packet's timestamp (already host-time units).
    func receiveExternalTick(hostTime: UInt64) {
        guard project.midi.clockSource == .external, isPlaying else { return }
        let minimumSafeTime = mach_absolute_time() &+ AVAudioTime.hostTime(forSeconds: 0.003)
        fireTick(globalTick, hostTime: max(hostTime, minimumSafeTime))
        updateExternalBPMEstimate(tickHostTime: hostTime)
        globalTick += 1
    }

    func externalStart() {
        guard project.midi.clockSource == .external else { return }
        DispatchQueue.main.async { self.beginPlaying() }
    }

    func externalContinue() {
        guard project.midi.clockSource == .external else { return }
        DispatchQueue.main.async { self.isPlaying = true }
    }

    func externalStop() {
        guard project.midi.clockSource == .external else { return }
        DispatchQueue.main.async {
            self.isPlaying = false
            self.audioEngine.stopAll()
        }
    }

    // MARK: - External BPM display

    private var lastExternalTickHostTime: UInt64?

    /// Smoothed BPM estimate purely for the transport display; the actual
    /// scheduling always uses the incoming clock's own host times, never
    /// this estimate.
    private func updateExternalBPMEstimate(tickHostTime: UInt64) {
        defer { lastExternalTickHostTime = tickHostTime }
        guard let last = lastExternalTickHostTime, tickHostTime > last else { return }
        let intervalSeconds = AVAudioTime.seconds(forHostTime: tickHostTime - last)
        guard intervalSeconds > 0 else { return }
        let instantaneousBPM = 60.0 / (intervalSeconds * Double(Sequencer.ticksPerQuarterNote))
        guard instantaneousBPM.isFinite, instantaneousBPM > 20, instantaneousBPM < 400 else { return }
        DispatchQueue.main.async {
            let smoothed = self.estimatedExternalBPM.map { $0 * 0.9 + instantaneousBPM * 0.1 } ?? instantaneousBPM
            self.estimatedExternalBPM = smoothed
        }
    }

    // MARK: - Pattern switching

    func queuePatternChange(_ index: Int) {
        guard index != project.activePatternIndex else { return }
        if isPlaying {
            pendingPatternIndex = index
        } else {
            project.activePatternIndex = index
        }
    }

    private func applyPendingPatternIfNeeded(atTick tick: Int) {
        guard tick % Sequencer.barLengthTicks == 0, pendingPatternIndex != nil else { return }
        DispatchQueue.main.async {
            guard let pending = self.pendingPatternIndex else { return }
            self.project.activePatternIndex = pending
            self.pendingPatternIndex = nil
        }
    }

    // MARK: - Tick handling

    private func fireTick(_ tick: Int, hostTime: UInt64) {
        applyPendingPatternIfNeeded(atTick: tick)
        let pattern = project.activePattern
        let sceneSetup = project.activeSceneSetup

        for (trackIndex, track) in project.tracks.enumerated() {
            let ticksPerStep = track.scale.ticksPerStep
            let cycleLength = ticksPerStep * max(1, track.stepCount)
            guard cycleLength > 0 else { continue }
            let positionInCycle = tick % cycleLength
            guard positionInCycle % ticksPerStep == 0 else { continue }
            let stepIndex = positionInCycle / ticksPerStep

            updatePlayhead(trackIndex: trackIndex, stepIndex: stepIndex)

            let step = pattern.step(track: trackIndex, index: stepIndex)
            guard let trig = step.trig, !track.isMuted else { continue }

            let adjustedHostTime = self.adjustedHostTime(hostTime, ticksPerStep: ticksPerStep, stepIndex: stepIndex, trig: trig, swing: pattern.swing)
            let resolved = resolveParameters(track: track, trackIndex: trackIndex, lock: trig.locks, sceneSetup: sceneSetup, velocity: trig.velocity)
            audioEngine.trigger(trackIndex: trackIndex, resolved: resolved, type: trig.type, loop: track.loop, hostTime: adjustedHostTime)

            if track.midiOut.enabled, trig.type == .note {
                let velocity = UInt8(max(1, min(127, Int(trig.velocity * 127))))
                midiManager?.sendNoteOn(channel: track.midiOut.channel, note: track.midiOut.note, velocity: velocity, atHostTime: adjustedHostTime)
            }
        }
    }

    private func adjustedHostTime(_ hostTime: UInt64, ticksPerStep: Int, stepIndex: Int, trig: Step.Trig, swing: Double) -> UInt64 {
        var offsetTicks = trig.microTiming * Double(ticksPerStep)
        if stepIndex % 2 == 1 {
            offsetTicks += swing * Double(ticksPerStep)
        }
        guard offsetTicks != 0 else { return hostTime }
        let offsetSeconds = offsetTicks * secondsPerTick
        let offsetHostTicks = AVAudioTime.hostTime(forSeconds: abs(offsetSeconds))
        return offsetSeconds >= 0 ? hostTime &+ offsetHostTicks : hostTime &- offsetHostTicks
    }

    private func updatePlayhead(trackIndex: Int, stepIndex: Int) {
        DispatchQueue.main.async { [weak self] in
            guard let self, self.currentStepPerTrack.indices.contains(trackIndex) else { return }
            self.currentStepPerTrack[trackIndex] = stepIndex
        }
    }

    /// Merges track base parameters, the active scene's crossfaded
    /// override, and this step's parameter locks (which always win) into
    /// one resolved parameter set ready for `TrackAudioChain`.
    func resolveParameters(track: Track, trackIndex: Int, lock: [ParameterID: Double], sceneSetup: SceneSetup, velocity: Double) -> [ParameterID: Double] {
        var resolved: [ParameterID: Double] = [:]
        for param in ParameterID.allCases {
            var value = track.parameters[param]
            if let morphed = sceneSetup.morphedValue(track: trackIndex, parameter: param) {
                value = morphed
            }
            if let locked = lock[param] {
                value = locked
            }
            resolved[param] = value
        }
        resolved[.volume] = (resolved[.volume] ?? ParameterID.volume.defaultValue) * velocity
        return resolved
    }

    /// Live (non-triggering) parameter push, e.g. from an incoming MIDI CC
    /// or the mixer UI. Applies scene morph on top but no p-locks (there is
    /// no "current step" outside of a trig).
    func applyLiveParameter(trackIndex: Int, parameter: ParameterID, value: Double) {
        guard project.tracks.indices.contains(trackIndex) else { return }
        project.tracks[trackIndex].parameters[parameter] = value
        let track = project.tracks[trackIndex]
        let resolved = resolveParameters(track: track, trackIndex: trackIndex, lock: [:], sceneSetup: project.activeSceneSetup, velocity: 1.0)
        audioEngine.applyLiveParameters(trackIndex: trackIndex, resolved: resolved)
    }
}
