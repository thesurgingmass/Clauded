import Foundation

/// Every value a step can parameter-lock (a "p-lock" in Octatrack terms).
/// Each case also carries the valid range and default, so the sequencer,
/// the MIDI CC map, and the UI knobs all agree on one source of truth.
enum ParameterID: String, CaseIterable, Codable, Identifiable {
    case volume
    case pan
    case pitch          // semitones, -24...24
    case sampleStart    // normalized 0...1 of sample length
    case sampleLength   // normalized 0...1 of remaining sample length
    case filterCutoff   // 0...1
    case filterResonance
    case delaySend
    case delayTime      // 0...1 mapped to musical divisions
    case delayFeedback

    var id: String { rawValue }

    var displayName: String {
        switch self {
        case .volume: return "Volume"
        case .pan: return "Pan"
        case .pitch: return "Pitch"
        case .sampleStart: return "Start"
        case .sampleLength: return "Length"
        case .filterCutoff: return "Filter Freq"
        case .filterResonance: return "Filter Reso"
        case .delaySend: return "Delay Send"
        case .delayTime: return "Delay Time"
        case .delayFeedback: return "Delay FB"
        }
    }

    var defaultValue: Double {
        switch self {
        case .volume: return 0.8
        case .pan: return 0.5
        case .pitch: return 0.5 // maps to 0 semitones, see TrackParameters
        case .sampleStart: return 0.0
        case .sampleLength: return 1.0
        case .filterCutoff: return 1.0
        case .filterResonance: return 0.0
        case .delaySend: return 0.0
        case .delayTime: return 0.375
        case .delayFeedback: return 0.3
        }
    }

    /// Parameters that make sense to trigger via an incoming MIDI CC by default.
    static var midiAssignable: [ParameterID] { allCases }
}
