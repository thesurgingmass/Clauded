import AVFoundation
import Foundation
import SwiftUI

/// The composition root: owns the audio engine, sequencer, MIDI I/O and
/// the current project, and wires MIDI messages into sequencer/track
/// actions. Views read/act through this via `@EnvironmentObject`.
final class AppState: ObservableObject {
    @Published private(set) var project: Project
    @Published private(set) var sequencer: Sequencer
    @Published var selectedTrackIndex: Int = 0
    @Published var selectedStepPage: Int = 0 // 16-step page within a track's up-to-64 steps
    @Published var errorMessage: String?

    let audioEngine: AudioEngine
    let midiManager = MIDIManager()
    private let projectStore = ProjectStore()

    init() {
        let project = Project()
        let engine = AudioEngine(trackCount: Project.trackCount)
        self.project = project
        self.audioEngine = engine
        self.sequencer = Sequencer(project: project, audioEngine: engine)
        self.sequencer.midiManager = midiManager

        do {
            try engine.start()
        } catch {
            errorMessage = "Couldn't start the audio engine: \(error.localizedDescription)"
        }

        wireMIDI()
    }

    // MARK: - MIDI wiring

    private func wireMIDI() {
        midiManager.onClockTick = { [weak self] hostTime in
            self?.sequencer.receiveExternalTick(hostTime: hostTime)
        }
        midiManager.onStart = { [weak self] in self?.sequencer.externalStart() }
        midiManager.onStop = { [weak self] in self?.sequencer.externalStop() }
        midiManager.onContinue = { [weak self] in self?.sequencer.externalContinue() }

        midiManager.onNoteOn = { [weak self] channel, note, velocity in
            guard let self else { return }
            for (index, track) in self.project.tracks.enumerated()
            where track.midiChannel == channel && track.midiBaseNote == note {
                let resolved = self.sequencer.resolveParameters(
                    track: track, trackIndex: index, lock: [:],
                    sceneSetup: self.project.activeSceneSetup,
                    velocity: Double(velocity) / 127.0
                )
                self.audioEngine.trigger(trackIndex: index, resolved: resolved, type: .note, loop: track.loop, hostTime: nil)
            }
        }

        midiManager.onNoteOff = { [weak self] channel, note in
            guard let self else { return }
            for (index, track) in self.project.tracks.enumerated()
            where track.midiChannel == channel && track.midiBaseNote == note && track.loop {
                self.audioEngine.chains[index].stop()
            }
        }

        midiManager.onCC = { [weak self] channel, controller, value in
            guard let self else { return }
            guard let mapping = self.project.midi.ccMappings.first(where: { $0.channel == channel && $0.controller == controller }) else { return }
            let normalized = Double(value) / 127.0
            DispatchQueue.main.async {
                self.sequencer.applyLiveParameter(trackIndex: mapping.trackIndex, parameter: mapping.parameter, value: normalized)
            }
        }

        midiManager.onProgramChange = { [weak self] _, program in
            guard let self, self.project.midi.programChangeSelectsPattern else { return }
            let index = Int(program) % Project.patternCount
            DispatchQueue.main.async { self.sequencer.queuePatternChange(index) }
        }

        midiManager.onMappingLearned = { [weak self] mapping in
            DispatchQueue.main.async { self?.project.midi.addOrReplaceMapping(mapping) }
        }
    }

    // MARK: - Track actions

    func loadSample(url: URL, forTrack trackIndex: Int) {
        do {
            _ = try audioEngine.loadSample(url: url, forTrack: trackIndex)
            project.tracks[trackIndex].sampleURL = url
            project.tracks[trackIndex].sampleBookmark = nil
        } catch {
            errorMessage = "Couldn't load sample: \(error.localizedDescription)"
        }
    }

    func previewTrack(_ trackIndex: Int) {
        guard project.tracks.indices.contains(trackIndex) else { return }
        let track = project.tracks[trackIndex]
        let resolved = sequencer.resolveParameters(track: track, trackIndex: trackIndex, lock: [:], sceneSetup: project.activeSceneSetup, velocity: 1.0)
        audioEngine.trigger(trackIndex: trackIndex, resolved: resolved, type: .note, loop: track.loop, hostTime: nil)
    }

    func setMuted(_ muted: Bool, trackIndex: Int) {
        guard project.tracks.indices.contains(trackIndex) else { return }
        project.tracks[trackIndex].isMuted = muted
        let resolvedVolume = project.tracks[trackIndex].parameters[.volume]
        audioEngine.setMuted(trackIndex: trackIndex, muted: muted, resolvedVolume: resolvedVolume)
    }

    // MARK: - Project lifecycle

    func newProject() {
        sequencer.stop()
        let fresh = Project()
        replaceProject(with: fresh)
    }

    func open(url: URL) {
        do {
            let loaded = try projectStore.load(from: url)
            sequencer.stop()
            replaceProject(with: loaded)
        } catch {
            errorMessage = "Couldn't open project: \(error.localizedDescription)"
        }
    }

    func save(to url: URL) {
        do {
            try projectStore.save(project, to: url)
        } catch {
            errorMessage = "Couldn't save project: \(error.localizedDescription)"
        }
    }

    func recentProjectURLs() -> [URL] { projectStore.recentProjectURLs() }

    /// Two-way binding into a single step of the active pattern, shared by
    /// the step grid and the parameter-lock panel so both edit the same
    /// underlying `Project.activePattern` storage.
    func stepBinding(track: Int, index: Int) -> Binding<Step> {
        Binding(
            get: { self.project.activePattern.step(track: track, index: index) },
            set: { newValue in
                var pattern = self.project.activePattern
                pattern.setStep(newValue, track: track, index: index)
                self.project.activePattern = pattern
            }
        )
    }

    private func replaceProject(with newProject: Project) {
        project = newProject
        sequencer = Sequencer(project: newProject, audioEngine: audioEngine)
        sequencer.midiManager = midiManager
        selectedTrackIndex = 0
        selectedStepPage = 0

        for (index, track) in newProject.tracks.enumerated() {
            audioEngine.chains[index].setAsset(nil)
            if let url = track.sampleURL {
                try? audioEngine.loadSample(url: url, forTrack: index)
            }
            let resolved = sequencer.resolveParameters(track: track, trackIndex: index, lock: [:], sceneSetup: newProject.activeSceneSetup, velocity: 1.0)
            audioEngine.setMuted(trackIndex: index, muted: track.isMuted, resolvedVolume: resolved[.volume] ?? 0.8)
            audioEngine.applyLiveParameters(trackIndex: index, resolved: resolved)
        }
    }
}
