#pragma once

#include <cmath>

namespace fm
{
    constexpr float twoPi = 6.283185307179586f;

    /** Converts an operator's 0..1 "level" into a phase-modulation index in
        radians when that operator is used as a modulator rather than a
        carrier. Chosen so level=1 gives a bright, harmonically dense result
        without excessive aliasing at typical audio ratios. */
    constexpr float defaultModulationIndexScale = 8.0f * 3.14159265358979323846f;

    /** Converts a MIDI note number to frequency in Hz (A4 = 69 = 440Hz). */
    inline double noteToFrequency(float midiNote)
    {
        return 440.0 * std::pow(2.0, (static_cast<double>(midiNote) - 69.0) / 12.0);
    }

    /** Converts a semitone offset to a frequency ratio multiplier. */
    inline double semitonesToRatio(double semitones)
    {
        return std::pow(2.0, semitones / 12.0);
    }

    inline float clamp01(float v)
    {
        return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
    }
}
