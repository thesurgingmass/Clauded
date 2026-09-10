import AVFoundation
import Foundation

/// The playback chain for one track:
/// player -> pitch -> filter (EQ) -> delay -> volume/pan mixer -> main mixer.
/// One instance per track; a track is monophonic (a new trig cuts
/// whatever it was already playing), matching Octatrack track behavior.
final class TrackAudioChain {
    let player = AVAudioPlayerNode()
    let pitch = AVAudioUnitTimePitch()
    let filter = AVAudioUnitEQ(numberOfBands: 1)
    let delay = AVAudioUnitDelay()
    let mixer = AVAudioMixerNode()

    private var currentAsset: SampleAsset?
    var isMuted: Bool = false

    init(engine: AVAudioEngine, format: AVAudioFormat) {
        filter.bands[0].filterType = .lowPass
        filter.bands[0].frequency = 20000
        filter.bands[0].bypass = false

        delay.wetDryMix = 0

        engine.attach(player)
        engine.attach(pitch)
        engine.attach(filter)
        engine.attach(delay)
        engine.attach(mixer)

        engine.connect(player, to: pitch, format: format)
        engine.connect(pitch, to: filter, format: format)
        engine.connect(filter, to: delay, format: format)
        engine.connect(delay, to: mixer, format: format)
        engine.connect(mixer, to: engine.mainMixerNode, format: format)
    }

    func setAsset(_ asset: SampleAsset?) {
        currentAsset = asset
    }

    var hasAsset: Bool { currentAsset != nil }

    /// Applies a fully-resolved parameter set (base values with any
    /// p-locks/scene morph already merged) and, for `.note` trigs,
    /// schedules the sample to start at `hostTime`.
    func trigger(resolved: [ParameterID: Double], type: TrigType, loop: Bool, hostTime: UInt64?) {
        apply(resolved: resolved)

        guard type == .note, let asset = currentAsset else { return }

        let start = resolved[.sampleStart] ?? ParameterID.sampleStart.defaultValue
        let length = resolved[.sampleLength] ?? ParameterID.sampleLength.defaultValue
        let buffer = asset.trimmedBuffer(start: start, length: max(0.001, length))

        let time: AVAudioTime? = hostTime.map { AVAudioTime(hostTime: $0) }
        let options: AVAudioPlayerNodeBufferOptions = loop ? [.interrupts, .loops] : [.interrupts]

        player.scheduleBuffer(buffer, at: time, options: options, completionHandler: nil)
        if !player.isPlaying {
            player.play()
        }
    }

    func stop() {
        player.stop()
    }

    /// Applies parameter values (0...1, except pitch which is -24...24
    /// semitones-normalized) to the actual audio unit controls.
    func apply(resolved: [ParameterID: Double]) {
        if let v = resolved[.volume] {
            mixer.outputVolume = isMuted ? 0 : Float(v)
        }
        if let v = resolved[.pan] {
            mixer.pan = Float((v * 2) - 1)
        }
        if let v = resolved[.pitch] {
            // 0.5 = no shift; full range +/-24 semitones = +/-2400 cents.
            pitch.pitch = Float((v - 0.5) * 2 * 2400)
        }
        if let v = resolved[.filterCutoff] {
            filter.bands[0].frequency = Self.logFrequency(from: v)
        }
        if let v = resolved[.filterResonance] {
            filter.bands[0].bandwidth = Float(0.05 + v * 4.95)
        }
        if let v = resolved[.delaySend] {
            delay.wetDryMix = Float(v * 100)
        }
        if let v = resolved[.delayTime] {
            delay.delayTime = 0.02 + v * 1.18
        }
        if let v = resolved[.delayFeedback] {
            delay.feedback = Float(v * 100) - 50 // AVAudioUnitDelay feedback is -100...100
        }
    }

    private static func logFrequency(from normalized: Double) -> Float {
        let minF = 20.0, maxF = 20000.0
        let v = pow(10, log10(minF) + normalized * (log10(maxF) - log10(minF)))
        return Float(v)
    }
}
