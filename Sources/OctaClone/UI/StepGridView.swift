import SwiftUI

struct StepGridView: View {
    @EnvironmentObject var appState: AppState
    @EnvironmentObject var project: Project

    var body: some View {
        // `.id(trackIndex)` gives a fresh StepGridContent (and fresh
        // @State selectedStepIndex) whenever the selected track changes,
        // and lets the content view observe *that* track's own @Published
        // properties directly via @ObservedObject.
        StepGridContent(trackIndex: appState.selectedTrackIndex, track: project.tracks[appState.selectedTrackIndex])
            .id(appState.selectedTrackIndex)
    }
}

private struct StepGridContent: View {
    @EnvironmentObject var appState: AppState
    @EnvironmentObject var project: Project
    @ObservedObject var track: Track
    let trackIndex: Int
    @State private var selectedStepIndex: Int = 0

    init(trackIndex: Int, track: Track) {
        self.trackIndex = trackIndex
        self.track = track
    }

    private var pattern: Pattern { project.activePattern }
    private var pageCount: Int { max(1, Int((Double(track.stepCount) / 16.0).rounded(.up))) }

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            HStack {
                Text("\(track.name) — \(pattern.name)").font(.headline)
                Spacer()
                Picker("Page", selection: $appState.selectedStepPage) {
                    ForEach(0..<pageCount, id: \.self) { p in
                        Text("Steps \(p * 16 + 1)-\(min(track.stepCount, p * 16 + 16))").tag(p)
                    }
                }
                .frame(width: 220)
                .onChange(of: pageCount) { newCount in
                    if appState.selectedStepPage >= newCount { appState.selectedStepPage = 0 }
                }
            }

            LazyVGrid(columns: Array(repeating: GridItem(.flexible(), spacing: 4), count: 16), spacing: 4) {
                ForEach(0..<16, id: \.self) { offset in
                    let index = appState.selectedStepPage * 16 + offset
                    if index < track.stepCount {
                        StepCell(trackIndex: trackIndex, stepIndex: index, isSelected: selectedStepIndex == index) {
                            selectedStepIndex = index
                            toggleTrig(index: index)
                        }
                    } else {
                        Color.clear.frame(height: 40)
                    }
                }
            }

            ParamLockPanel(trackIndex: trackIndex, stepIndex: selectedStepIndex)
        }
    }

    private func toggleTrig(index: Int) {
        let binding = appState.stepBinding(track: trackIndex, index: index)
        var step = binding.wrappedValue
        step.trig = step.isEmpty ? Step.note().trig : nil
        binding.wrappedValue = step
    }
}

private struct StepCell: View {
    @EnvironmentObject var project: Project
    @EnvironmentObject var sequencer: Sequencer
    let trackIndex: Int
    let stepIndex: Int
    let isSelected: Bool
    let action: () -> Void

    private var step: Step {
        project.activePattern.step(track: trackIndex, index: stepIndex)
    }

    private var isPlayhead: Bool {
        sequencer.isPlaying && sequencer.currentStepPerTrack[safe: trackIndex] == stepIndex
    }

    var body: some View {
        Button(action: action) {
            ZStack(alignment: .bottomTrailing) {
                RoundedRectangle(cornerRadius: 4)
                    .fill(fillColor)
                Text("\(stepIndex + 1)")
                    .font(.system(size: 9))
                    .foregroundStyle(.secondary)
                    .padding(2)
            }
            .frame(height: 40)
            .overlay(
                RoundedRectangle(cornerRadius: 4)
                    .stroke(isSelected ? Color.white : .clear, lineWidth: 2)
            )
        }
        .buttonStyle(.plain)
    }

    private var fillColor: Color {
        if isPlayhead { return .green }
        guard let trig = step.trig else {
            return Color.gray.opacity(stepIndex % 4 == 0 ? 0.28 : 0.15)
        }
        return trig.type == .lock ? .purple : .accentColor
    }
}
