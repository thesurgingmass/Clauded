import AppKit
import SwiftUI
import UniformTypeIdentifiers

struct TrackStripsView: View {
    @EnvironmentObject var appState: AppState

    var body: some View {
        ScrollView {
            VStack(spacing: 6) {
                ForEach(Array(appState.project.tracks.enumerated()), id: \.element.id) { index, track in
                    TrackStripRow(trackIndex: index)
                        .environmentObject(track)
                }
            }
            .padding(10)
        }
    }
}

private struct TrackStripRow: View {
    @EnvironmentObject var appState: AppState
    @EnvironmentObject var sequencer: Sequencer
    @EnvironmentObject var track: Track
    let trackIndex: Int

    private var isSelected: Bool { appState.selectedTrackIndex == trackIndex }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            HStack {
                Button {
                    appState.selectedTrackIndex = trackIndex
                } label: {
                    Text(track.name)
                        .fontWeight(isSelected ? .bold : .regular)
                }
                .buttonStyle(.plain)

                Spacer()

                Toggle(isOn: Binding(
                    get: { track.isMuted },
                    set: { appState.setMuted($0, trackIndex: trackIndex) }
                )) {
                    Text("M").font(.caption2)
                }
                .toggleStyle(.button)

                Button {
                    appState.previewTrack(trackIndex)
                } label: {
                    Image(systemName: "play.circle")
                }
                .buttonStyle(.plain)
            }

            HStack {
                Button(track.sampleURL?.lastPathComponent ?? "Load Sample…") {
                    chooseSample()
                }
                .font(.caption)
                .lineLimit(1)
                .truncationMode(.middle)
            }

            HStack {
                Text("Vol").font(.caption2).foregroundStyle(.secondary)
                Slider(value: Binding(
                    get: { track.parameters[.volume] },
                    set: { sequencer.applyLiveParameter(trackIndex: trackIndex, parameter: .volume, value: $0) }
                ))
            }

            HStack(spacing: 10) {
                Stepper("Ch \(track.midiChannel + 1)", value: Binding(
                    get: { track.midiChannel },
                    set: { track.midiChannel = $0 }
                ), in: 0...15)
                .font(.caption2)

                Stepper("Note \(track.midiBaseNote)", value: Binding(
                    get: { track.midiBaseNote },
                    set: { track.midiBaseNote = $0 }
                ), in: 0...127)
                .font(.caption2)
            }

            HStack(spacing: 10) {
                Picker("", selection: Binding(get: { track.scale }, set: { track.scale = $0 })) {
                    ForEach(TrackScale.allCases) { scale in
                        Text(scale.displayName).tag(scale)
                    }
                }
                .frame(width: 70)
                .labelsHidden()

                Stepper("\(track.stepCount) steps", value: Binding(
                    get: { track.stepCount },
                    set: { track.stepCount = $0 }
                ), in: 1...Pattern.maxSteps)
                .font(.caption2)
            }
        }
        .padding(8)
        .background(isSelected ? Color.accentColor.opacity(0.15) : Color.gray.opacity(0.08))
        .cornerRadius(6)
        .contentShape(Rectangle())
        .onTapGesture { appState.selectedTrackIndex = trackIndex }
    }

    private func chooseSample() {
        let panel = NSOpenPanel()
        panel.allowedContentTypes = [.audio]
        panel.canChooseDirectories = false
        panel.begin { response in
            guard response == .OK, let url = panel.url else { return }
            appState.loadSample(url: url, forTrack: trackIndex)
        }
    }
}

extension Array {
    subscript(safe index: Int) -> Element? {
        indices.contains(index) ? self[index] : nil
    }
}
