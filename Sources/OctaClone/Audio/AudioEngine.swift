import AVFoundation

/// Owns the AVAudioEngine graph and one `TrackAudioChain` per track.
final class AudioEngine {
    let engine = AVAudioEngine()
    let format: AVAudioFormat
    private(set) var chains: [TrackAudioChain] = []
    private var assets: [Int: SampleAsset] = [:] // trackIndex -> loaded sample

    init(trackCount: Int) {
        format = AVAudioFormat(standardFormatWithSampleRate: 44100, channels: 2)!
        for _ in 0..<trackCount {
            chains.append(TrackAudioChain(engine: engine, format: format))
        }
        engine.mainMixerNode.outputVolume = 1.0
    }

    func start() throws {
        engine.prepare()
        try engine.start()
    }

    func stop() {
        engine.stop()
    }

    @discardableResult
    func loadSample(url: URL, forTrack trackIndex: Int) throws -> SampleAsset {
        let asset = try SampleAsset(url: url, targetFormat: format)
        assets[trackIndex] = asset
        chains[trackIndex].setAsset(asset)
        return asset
    }

    func asset(forTrack trackIndex: Int) -> SampleAsset? { assets[trackIndex] }

    func trigger(trackIndex: Int, resolved: [ParameterID: Double], type: TrigType, loop: Bool, hostTime: UInt64?) {
        guard chains.indices.contains(trackIndex) else { return }
        chains[trackIndex].trigger(resolved: resolved, type: type, loop: loop, hostTime: hostTime)
    }

    /// Applies live parameter changes (e.g. from an incoming MIDI CC)
    /// without retriggering the sample.
    func applyLiveParameters(trackIndex: Int, resolved: [ParameterID: Double]) {
        guard chains.indices.contains(trackIndex) else { return }
        chains[trackIndex].apply(resolved: resolved)
    }

    func setMuted(trackIndex: Int, muted: Bool, resolvedVolume: Double) {
        guard chains.indices.contains(trackIndex) else { return }
        chains[trackIndex].isMuted = muted
        chains[trackIndex].mixer.outputVolume = muted ? 0 : Float(resolvedVolume)
    }

    func stopAll() {
        chains.forEach { $0.stop() }
    }
}
