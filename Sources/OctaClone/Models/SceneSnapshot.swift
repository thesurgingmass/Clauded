import Foundation

/// A saved set of per-track parameter values that the crossfader morphs
/// between (Octatrack "Scene A" / "Scene B"). Only parameters explicitly
/// stored here are affected by the crossfader; everything else stays at
/// the track's own base value.
struct SceneSnapshot: Codable, Equatable {
    /// values[trackIndex][parameter] = value
    var values: [[ParameterID: Double]]

    init(trackCount: Int) {
        values = Array(repeating: [:], count: trackCount)
    }

    func value(track: Int, parameter: ParameterID) -> Double? {
        guard values.indices.contains(track) else { return nil }
        return values[track][parameter]
    }

    mutating func set(track: Int, parameter: ParameterID, value: Double?) {
        guard values.indices.contains(track) else { return }
        values[track][parameter] = value
    }
}

/// Owns the A/B snapshots and the crossfader position for one pattern's
/// scene morphing.
struct SceneSetup: Codable, Equatable {
    var sceneA: SceneSnapshot
    var sceneB: SceneSnapshot
    /// 0 = full A, 1 = full B.
    var crossfaderPosition: Double = 0.5

    init(trackCount: Int) {
        sceneA = SceneSnapshot(trackCount: trackCount)
        sceneB = SceneSnapshot(trackCount: trackCount)
    }

    /// Linear-interpolated override for a track/parameter, or nil if neither
    /// scene locks that parameter (meaning the track's base value applies).
    func morphedValue(track: Int, parameter: ParameterID) -> Double? {
        let a = sceneA.value(track: track, parameter: parameter)
        let b = sceneB.value(track: track, parameter: parameter)
        switch (a, b) {
        case let (a?, b?):
            return a + (b - a) * crossfaderPosition
        case let (a?, nil):
            return a * (1 - crossfaderPosition)
        case let (nil, b?):
            return b * crossfaderPosition
        case (nil, nil):
            return nil
        }
    }
}
