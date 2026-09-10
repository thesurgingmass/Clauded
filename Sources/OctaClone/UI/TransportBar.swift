import SwiftUI

struct TransportBar: View {
    @EnvironmentObject var project: Project
    @EnvironmentObject var sequencer: Sequencer
    @Binding var showingMIDISettings: Bool

    private var bpmBinding: Binding<Double> {
        Binding(get: { project.bpm }, set: { project.bpm = $0 })
    }

    var body: some View {
        HStack(spacing: 20) {
            Button {
                sequencer.togglePlay()
            } label: {
                Image(systemName: sequencer.isPlaying ? "stop.fill" : "play.fill")
                    .font(.title2)
                    .frame(width: 32, height: 32)
            }
            .disabled(project.midi.clockSource == .external)
            .help(project.midi.clockSource == .external ? "Transport is controlled by the external MIDI clock" : "Play/Stop")

            VStack(alignment: .leading, spacing: 2) {
                Text("BPM")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                Stepper(value: bpmBinding, in: 40...300, step: 1) {
                    Text(String(format: "%.0f", project.bpm))
                        .monospacedDigit()
                        .frame(width: 40, alignment: .leading)
                }
            }
            .disabled(project.midi.clockSource == .external)

            if project.midi.clockSource == .external, let estimated = sequencer.estimatedExternalBPM {
                VStack(alignment: .leading, spacing: 2) {
                    Text("EXT BPM").font(.caption).foregroundStyle(.secondary)
                    Text(String(format: "%.0f", estimated)).monospacedDigit()
                }
            }

            PatternGrid()

            Spacer()

            Picker("Clock", selection: Binding(
                get: { project.midi.clockSource },
                set: { project.midi.clockSource = $0 }
            )) {
                ForEach(ClockSource.allCases) { source in
                    Text(source.displayName).tag(source)
                }
            }
            .frame(width: 220)

            Button {
                showingMIDISettings = true
            } label: {
                Label("MIDI", systemImage: "pianokeys")
            }
        }
    }
}

private struct PatternGrid: View {
    @EnvironmentObject var project: Project
    @EnvironmentObject var sequencer: Sequencer
    private let columns = Array(repeating: GridItem(.fixed(22), spacing: 3), count: 8)

    var body: some View {
        LazyVGrid(columns: columns, spacing: 3) {
            ForEach(0..<Project.patternCount, id: \.self) { index in
                Button {
                    sequencer.queuePatternChange(index)
                } label: {
                    Text("\(index + 1)")
                        .font(.system(size: 10, weight: .medium))
                        .frame(width: 22, height: 18)
                        .background(background(for: index))
                        .cornerRadius(3)
                }
                .buttonStyle(.plain)
            }
        }
    }

    private func background(for index: Int) -> Color {
        if index == sequencer.pendingPatternIndex { return .orange }
        if index == project.activePatternIndex { return .accentColor }
        return Color.gray.opacity(0.25)
    }
}
