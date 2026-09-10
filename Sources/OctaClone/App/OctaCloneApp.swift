import AppKit
import SwiftUI

@main
struct OctaCloneApp: App {
    @StateObject private var appState = AppState()

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environmentObject(appState)
                .environmentObject(appState.project)
                .environmentObject(appState.sequencer)
                .environmentObject(appState.midiManager)
                .frame(minWidth: 980, minHeight: 640)
        }
        .windowResizability(.contentSize)
        .commands {
            CommandGroup(replacing: .newItem) {
                Button("New Project") { appState.newProject() }
                    .keyboardShortcut("n", modifiers: .command)
                Button("Open Project…") { openProjectPanel() }
                    .keyboardShortcut("o", modifiers: .command)
                Button("Save Project…") { saveProjectPanel() }
                    .keyboardShortcut("s", modifiers: .command)
            }
        }
    }

    private func openProjectPanel() {
        let panel = NSOpenPanel()
        panel.allowedContentTypes = []
        panel.allowsOtherFileTypes = true
        panel.canChooseDirectories = false
        panel.begin { response in
            guard response == .OK, let url = panel.url else { return }
            appState.open(url: url)
        }
    }

    private func saveProjectPanel() {
        let panel = NSSavePanel()
        panel.nameFieldStringValue = "\(appState.project.name).\(ProjectStore.fileExtension)"
        panel.begin { response in
            guard response == .OK, let url = panel.url else { return }
            appState.save(to: url)
        }
    }
}
