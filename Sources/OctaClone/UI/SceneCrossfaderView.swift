import SwiftUI

/// Octatrack-style scene morphing: Scene A and Scene B each snapshot a
/// track's parameters, and the crossfader linearly morphs between them
/// live during playback (see `SceneSnapshot.morphedValue`).
struct SceneCrossfaderView: View {
    @EnvironmentObject var appState: AppState
    @EnvironmentObject var project: Project

    private var sceneSetupBinding: Binding<SceneSetup> {
        Binding(
            get: { project.activeSceneSetup },
            set: { project.activeSceneSetup = $0 }
        )
    }

    var body: some View {
        VStack(spacing: 8) {
            HStack {
                Text("Scene A").font(.caption).foregroundStyle(.secondary)
                Slider(value: sceneSetupBinding.crossfaderPosition, in: 0...1)
                Text("Scene B").font(.caption).foregroundStyle(.secondary)
            }

            HStack(spacing: 12) {
                Button("Store T\(appState.selectedTrackIndex + 1) → A") {
                    storeSelectedTrack(into: \.sceneA)
                }
                Button("Store T\(appState.selectedTrackIndex + 1) → B") {
                    storeSelectedTrack(into: \.sceneB)
                }
                Button("Clear T\(appState.selectedTrackIndex + 1) from Scenes") {
                    clearSelectedTrackFromScenes()
                }
                Spacer()
            }
            .font(.caption)
        }
    }

    private func storeSelectedTrack(into keyPath: WritableKeyPath<SceneSetup, SceneSnapshot>) {
        var setup = sceneSetupBinding.wrappedValue
        let track = project.tracks[appState.selectedTrackIndex]
        for parameter in ParameterID.allCases {
            setup[keyPath: keyPath].set(track: appState.selectedTrackIndex, parameter: parameter, value: track.parameters[parameter])
        }
        sceneSetupBinding.wrappedValue = setup
    }

    private func clearSelectedTrackFromScenes() {
        var setup = sceneSetupBinding.wrappedValue
        for parameter in ParameterID.allCases {
            setup.sceneA.set(track: appState.selectedTrackIndex, parameter: parameter, value: nil)
            setup.sceneB.set(track: appState.selectedTrackIndex, parameter: parameter, value: nil)
        }
        sceneSetupBinding.wrappedValue = setup
    }
}
