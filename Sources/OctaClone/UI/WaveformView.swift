import SwiftUI

/// Shows the selected track's sample with draggable start/end trim
/// handles bound live to the `.sampleStart` / `.sampleLength` parameters.
struct WaveformView: View {
    @EnvironmentObject var appState: AppState
    @EnvironmentObject var project: Project

    var body: some View {
        // Re-built whenever the selected track changes so the inner view
        // observes the *current* track's own @Published parameters directly.
        WaveformContent(trackIndex: appState.selectedTrackIndex, track: project.tracks[appState.selectedTrackIndex])
    }
}

private struct WaveformContent: View {
    @EnvironmentObject var appState: AppState
    @ObservedObject var track: Track
    let trackIndex: Int

    init(trackIndex: Int, track: Track) {
        self.trackIndex = trackIndex
        self.track = track
    }

    var body: some View {
        GeometryReader { geo in
            ZStack(alignment: .leading) {
                RoundedRectangle(cornerRadius: 6).fill(Color.black.opacity(0.85))

                if let peaks = appState.audioEngine.asset(forTrack: trackIndex)?.peaks(resolution: 400) {
                    WaveformShape(peaks: peaks)
                        .stroke(Color.accentColor, lineWidth: 1)
                        .padding(.vertical, 8)

                    let start = track.parameters[.sampleStart]
                    let length = track.parameters[.sampleLength]
                    let startX = geo.size.width * CGFloat(start)
                    let endX = geo.size.width * CGFloat(min(1, start + length))

                    Rectangle()
                        .fill(Color.white.opacity(0.15))
                        .frame(width: max(0, endX - startX))
                        .offset(x: startX)

                    TrimHandle()
                        .offset(x: startX - 2)
                        .gesture(dragStart(width: geo.size.width))
                    TrimHandle()
                        .offset(x: endX - 2)
                        .gesture(dragEnd(width: geo.size.width, start: start))
                } else {
                    Text("No sample loaded")
                        .foregroundStyle(.secondary)
                        .frame(maxWidth: .infinity, maxHeight: .infinity)
                }
            }
        }
    }

    private func dragStart(width: CGFloat) -> some Gesture {
        DragGesture(minimumDistance: 0).onChanged { value in
            guard width > 0 else { return }
            let newStart = max(0, min(1, Double(value.location.x / width)))
            appState.sequencer.applyLiveParameter(trackIndex: trackIndex, parameter: .sampleStart, value: newStart)
        }
    }

    private func dragEnd(width: CGFloat, start: Double) -> some Gesture {
        DragGesture(minimumDistance: 0).onChanged { value in
            guard width > 0 else { return }
            let endFraction = max(0, min(1, Double(value.location.x / width)))
            let newLength = max(0.01, endFraction - start)
            appState.sequencer.applyLiveParameter(trackIndex: trackIndex, parameter: .sampleLength, value: newLength)
        }
    }
}

private struct TrimHandle: View {
    var body: some View {
        // Mouse-driven (macOS), so a slim precise bar is fine as the hit target.
        Rectangle()
            .fill(Color.yellow)
            .frame(width: 4)
    }
}

private struct WaveformShape: Shape {
    let peaks: [Float]

    func path(in rect: CGRect) -> Path {
        var path = Path()
        guard !peaks.isEmpty else { return path }
        let midY = rect.midY
        let stepX = rect.width / CGFloat(peaks.count)
        for (i, peak) in peaks.enumerated() {
            let x = CGFloat(i) * stepX
            let halfHeight = CGFloat(peak) * rect.height / 2
            path.move(to: CGPoint(x: x, y: midY - halfHeight))
            path.addLine(to: CGPoint(x: x, y: midY + halfHeight))
        }
        return path
    }
}
