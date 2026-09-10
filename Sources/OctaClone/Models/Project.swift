import Foundation

final class Project: ObservableObject, Codable {
    static let trackCount = 8
    static let patternCount = 16

    @Published var name: String
    @Published var bpm: Double
    @Published var tracks: [Track]
    @Published var patterns: [Pattern]
    @Published var sceneSetups: [SceneSetup]   // one per pattern
    @Published var activePatternIndex: Int
    @Published var midi: MIDISettings

    init(name: String = "New Project") {
        self.name = name
        self.bpm = 120
        self.tracks = (0..<Project.trackCount).map { Track(index: $0) }
        self.patterns = (0..<Project.patternCount).map { Pattern(name: "P\($0 + 1)", trackCount: Project.trackCount) }
        self.sceneSetups = (0..<Project.patternCount).map { _ in SceneSetup(trackCount: Project.trackCount) }
        self.activePatternIndex = 0
        self.midi = MIDISettings()
    }

    var activePattern: Pattern {
        get { patterns[activePatternIndex] }
        set { patterns[activePatternIndex] = newValue }
    }

    var activeSceneSetup: SceneSetup {
        get { sceneSetups[activePatternIndex] }
        set { sceneSetups[activePatternIndex] = newValue }
    }

    // MARK: Codable

    enum CodingKeys: String, CodingKey {
        case name, bpm, tracks, patterns, sceneSetups, activePatternIndex, midi
    }

    convenience init(from decoder: Decoder) throws {
        self.init()
        let c = try decoder.container(keyedBy: CodingKeys.self)
        name = try c.decode(String.self, forKey: .name)
        bpm = try c.decode(Double.self, forKey: .bpm)
        tracks = try c.decode([Track].self, forKey: .tracks)
        patterns = try c.decode([Pattern].self, forKey: .patterns)
        sceneSetups = try c.decode([SceneSetup].self, forKey: .sceneSetups)
        activePatternIndex = try c.decode(Int.self, forKey: .activePatternIndex)
        midi = try c.decode(MIDISettings.self, forKey: .midi)
    }

    func encode(to encoder: Encoder) throws {
        var c = encoder.container(keyedBy: CodingKeys.self)
        try c.encode(name, forKey: .name)
        try c.encode(bpm, forKey: .bpm)
        try c.encode(tracks, forKey: .tracks)
        try c.encode(patterns, forKey: .patterns)
        try c.encode(sceneSetups, forKey: .sceneSetups)
        try c.encode(activePatternIndex, forKey: .activePatternIndex)
        try c.encode(midi, forKey: .midi)
    }
}
