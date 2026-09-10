import AVFoundation

/// A sample loaded fully into RAM (the Octatrack "flex machine" model —
/// there is no streamed "static machine" in this implementation, see
/// README roadmap).
final class SampleAsset {
    let url: URL
    let buffer: AVAudioPCMBuffer
    let sampleRate: Double

    var frameCount: AVAudioFrameCount { buffer.frameLength }
    var duration: TimeInterval { Double(frameCount) / sampleRate }

    /// - Parameter targetFormat: the format shared by the whole audio graph.
    ///   The file is converted to this format on load so every buffer
    ///   scheduled on a player node matches its connection format exactly.
    init(url: URL, targetFormat: AVAudioFormat) throws {
        let file = try AVAudioFile(forReading: url)
        let sourceFormat = file.processingFormat

        guard let rawBuffer = AVAudioPCMBuffer(pcmFormat: sourceFormat, frameCapacity: AVAudioFrameCount(file.length)) else {
            throw SampleAssetError.bufferAllocationFailed
        }
        try file.read(into: rawBuffer)

        if sourceFormat == targetFormat {
            self.buffer = rawBuffer
        } else {
            guard let converter = AVAudioConverter(from: sourceFormat, to: targetFormat) else {
                throw SampleAssetError.conversionFailed
            }
            let ratio = targetFormat.sampleRate / sourceFormat.sampleRate
            let outCapacity = AVAudioFrameCount(Double(rawBuffer.frameLength) * ratio) + 1024
            guard let converted = AVAudioPCMBuffer(pcmFormat: targetFormat, frameCapacity: outCapacity) else {
                throw SampleAssetError.bufferAllocationFailed
            }
            var errorPtr: NSError?
            var suppliedRaw = false
            converter.convert(to: converted, error: &errorPtr) { _, outStatus in
                if suppliedRaw {
                    outStatus.pointee = .noDataNow
                    return nil
                }
                suppliedRaw = true
                outStatus.pointee = .haveData
                return rawBuffer
            }
            if let errorPtr { throw errorPtr }
            self.buffer = converted
        }
        self.url = url
        self.sampleRate = targetFormat.sampleRate
    }

    /// Downsamples channel 0 into `resolution` peak (max-abs) buckets, for
    /// drawing a lightweight waveform view without touching the full buffer
    /// every frame.
    func peaks(resolution: Int) -> [Float] {
        guard let channelData = buffer.floatChannelData, frameCount > 0, resolution > 0 else { return [] }
        let samples = channelData[0]
        let total = Int(frameCount)
        let bucketSize = max(1, total / resolution)
        var result: [Float] = []
        result.reserveCapacity(resolution)
        var i = 0
        while i < total {
            let end = min(total, i + bucketSize)
            var peak: Float = 0
            for j in i..<end {
                peak = max(peak, abs(samples[j]))
            }
            result.append(peak)
            i = end
        }
        return result
    }

    /// Returns a sub-buffer covering `[start, start+length)` (both 0...1,
    /// fractions of the full sample) sharing the same underlying format.
    func trimmedBuffer(start: Double, length: Double) -> AVAudioPCMBuffer {
        let total = Int(frameCount)
        guard total > 0 else { return buffer }
        let startFrame = min(max(0, Int(Double(total) * start)), total - 1)
        let requestedLength = max(1, Int(Double(total) * length))
        let endFrame = min(total, startFrame + requestedLength)
        let outLength = AVAudioFrameCount(max(1, endFrame - startFrame))

        guard let out = AVAudioPCMBuffer(pcmFormat: buffer.format, frameCapacity: outLength) else {
            return buffer
        }
        out.frameLength = outLength

        let channelCount = Int(buffer.format.channelCount)
        if let src = buffer.floatChannelData, let dst = out.floatChannelData {
            for ch in 0..<channelCount {
                dst[ch].update(from: src[ch] + startFrame, count: Int(outLength))
            }
        }
        return out
    }
}

enum SampleAssetError: Error {
    case bufferAllocationFailed
    case conversionFailed
}
