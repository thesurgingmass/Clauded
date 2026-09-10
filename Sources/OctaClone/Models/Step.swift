import Foundation

/// What a trig does when the sequencer reaches it.
enum TrigType: String, Codable {
    /// Normal trig: (re)starts sample playback.
    case note
    /// Lock trig: applies parameter locks without retriggering the sample
    /// (Octatrack calls this a "lock trig").
    case lock
}

/// One step in a track's pattern. A step with `trig == nil` is empty.
struct Step: Codable, Equatable {
    struct Trig: Codable, Equatable {
        var type: TrigType = .note
        /// Per-step velocity/volume multiplier (0...1), independent of any
        /// explicit `.volume` parameter lock.
        var velocity: Double = 1.0
        /// Micro-timing offset in fractions of a step, -0.5...0.5.
        var microTiming: Double = 0.0
        /// Parameter locks held on this step. Only parameters present here
        /// override the track's base value for the duration of the step.
        var locks: [ParameterID: Double] = [:]
    }

    var trig: Trig?

    var isEmpty: Bool { trig == nil }

    static let empty = Step(trig: nil)

    static func note() -> Step { Step(trig: Trig()) }
}
