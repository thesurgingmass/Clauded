import Foundation

/// How many sequencer ticks make up one step, at 24 PPQN.
/// 6 ticks = a 16th note (the Octatrack default "1x" scale).
enum TrackScale: Int, Codable, CaseIterable, Identifiable {
    case doubleTime = 3   // 32nd notes: steps play twice as fast
    case normal = 6        // 16th notes
    case halfTime = 12     // 8th notes: steps play half as fast
    case quarterTime = 24  // quarter notes

    var id: Int { rawValue }
    var ticksPerStep: Int { rawValue }

    var displayName: String {
        switch self {
        case .doubleTime: return "2x"
        case .normal: return "1x"
        case .halfTime: return "1/2x"
        case .quarterTime: return "1/4x"
        }
    }
}

/// Base (unlocked) parameter values for a track, stored 0...1 (or the
/// natural range documented on `ParameterID`) and turned into real audio
/// unit values by `TrackAudioChain`.
struct TrackParameters: Codable, Equatable {
    var values: [ParameterID: Double]

    init() {
        values = Dictionary(uniqueKeysWithValues: ParameterID.allCases.map { ($0, $0.defaultValue) })
    }

    subscript(id: ParameterID) -> Double {
        get { values[id] ?? id.defaultValue }
        set { values[id] = newValue }
    }
}

struct MIDIOutConfig: Codable, Equatable {
    var enabled: Bool = false
    var channel: Int = 0       // 0...15
    var note: Int = 36         // note sent when the track triggers
}

final class Track: Identifiable, ObservableObject, Codable {
    let id: UUID
    var index: Int
    @Published var name: String
    @Published var sampleURL: URL?
    @Published var sampleBookmark: Data?
    @Published var parameters: TrackParameters
    @Published var scale: TrackScale
    @Published var stepCount: Int          // 1...64, independent per track
    @Published var isMuted: Bool
    @Published var loop: Bool               // loop sample vs one-shot
    @Published var midiChannel: Int         // channel this track listens on for note/CC in, 0...15
    @Published var midiBaseNote: Int        // incoming note number that triggers this track
    @Published var midiOut: MIDIOutConfig

    init(index: Int) {
        self.id = UUID()
        self.index = index
        self.name = "T\(index + 1)"
        self.sampleURL = nil
        self.sampleBookmark = nil
        self.parameters = TrackParameters()
        self.scale = .normal
        self.stepCount = 16
        self.isMuted = false
        self.loop = false
        self.midiChannel = 0
        self.midiBaseNote = 36 + index
        self.midiOut = MIDIOutConfig()
    }

    // MARK: Codable (ObservableObject classes need manual conformance)

    enum CodingKeys: String, CodingKey {
        case id, index, name, sampleURL, sampleBookmark, parameters, scale, stepCount, isMuted, loop, midiChannel, midiBaseNote, midiOut
    }

    required init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        id = try c.decode(UUID.self, forKey: .id)
        index = try c.decode(Int.self, forKey: .index)
        name = try c.decode(String.self, forKey: .name)
        sampleURL = try c.decodeIfPresent(URL.self, forKey: .sampleURL)
        sampleBookmark = try c.decodeIfPresent(Data.self, forKey: .sampleBookmark)
        parameters = try c.decode(TrackParameters.self, forKey: .parameters)
        scale = try c.decode(TrackScale.self, forKey: .scale)
        stepCount = try c.decode(Int.self, forKey: .stepCount)
        isMuted = try c.decode(Bool.self, forKey: .isMuted)
        loop = try c.decode(Bool.self, forKey: .loop)
        midiChannel = try c.decode(Int.self, forKey: .midiChannel)
        midiBaseNote = try c.decode(Int.self, forKey: .midiBaseNote)
        midiOut = try c.decode(MIDIOutConfig.self, forKey: .midiOut)
    }

    func encode(to encoder: Encoder) throws {
        var c = encoder.container(keyedBy: CodingKeys.self)
        try c.encode(id, forKey: .id)
        try c.encode(index, forKey: .index)
        try c.encode(name, forKey: .name)
        try c.encodeIfPresent(sampleURL, forKey: .sampleURL)
        try c.encodeIfPresent(sampleBookmark, forKey: .sampleBookmark)
        try c.encode(parameters, forKey: .parameters)
        try c.encode(scale, forKey: .scale)
        try c.encode(stepCount, forKey: .stepCount)
        try c.encode(isMuted, forKey: .isMuted)
        try c.encode(loop, forKey: .loop)
        try c.encode(midiChannel, forKey: .midiChannel)
        try c.encode(midiBaseNote, forKey: .midiBaseNote)
        try c.encode(midiOut, forKey: .midiOut)
    }
}
