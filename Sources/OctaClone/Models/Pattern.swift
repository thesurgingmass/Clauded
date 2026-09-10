import Foundation

/// A pattern holds one step lane per track. Each lane is allocated at the
/// maximum length (64) so a track's `stepCount` can be changed later
/// without losing steps beyond the new length.
struct Pattern: Codable, Identifiable, Equatable {
    static let maxSteps = 64

    var id: UUID = UUID()
    var name: String
    /// steps[trackIndex][stepIndex]
    var steps: [[Step]]
    /// Per-pattern swing, applied to odd-numbered steps as a fraction of a step (0...0.5).
    var swing: Double = 0.0

    init(name: String, trackCount: Int) {
        self.name = name
        self.steps = Array(repeating: Array(repeating: .empty, count: Pattern.maxSteps), count: trackCount)
    }

    func step(track: Int, index: Int) -> Step {
        guard steps.indices.contains(track), steps[track].indices.contains(index) else { return .empty }
        return steps[track][index]
    }

    mutating func setStep(_ step: Step, track: Int, index: Int) {
        guard steps.indices.contains(track), steps[track].indices.contains(index) else { return }
        steps[track][index] = step
    }

    mutating func toggleTrig(track: Int, index: Int) {
        guard steps.indices.contains(track), steps[track].indices.contains(index) else { return }
        if steps[track][index].isEmpty {
            steps[track][index] = .note()
        } else {
            steps[track][index] = .empty
        }
    }
}
