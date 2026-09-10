import SwiftUI

struct ContentView: View {
    @EnvironmentObject var appState: AppState
    @EnvironmentObject var project: Project
    @EnvironmentObject var sequencer: Sequencer
    @EnvironmentObject var midiManager: MIDIManager
    @State private var showingMIDISettings = false

    var body: some View {
        VStack(spacing: 0) {
            TransportBar(showingMIDISettings: $showingMIDISettings)
                .padding(12)
                .background(.regularMaterial)

            Divider()

            HStack(spacing: 0) {
                TrackStripsView()
                    .frame(width: 300)

                Divider()

                VStack(spacing: 0) {
                    WaveformView()
                        .frame(height: 110)
                        .padding(12)

                    Divider()

                    StepGridView()
                        .padding(12)

                    Divider()

                    SceneCrossfaderView()
                        .padding(12)
                }
            }
        }
        .sheet(isPresented: $showingMIDISettings) {
            MIDISettingsView()
                .environmentObject(appState)
                .environmentObject(project)
                .environmentObject(sequencer)
                .environmentObject(midiManager)
                .frame(minWidth: 560, minHeight: 480)
        }
        .alert("Error", isPresented: Binding(
            get: { appState.errorMessage != nil },
            set: { if !$0 { appState.errorMessage = nil } }
        )) {
            Button("OK", role: .cancel) { appState.errorMessage = nil }
        } message: {
            Text(appState.errorMessage ?? "")
        }
    }
}
