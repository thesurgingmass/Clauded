#pragma once

namespace vantage
{
    /**
     * Polynomial band-limited step (Valimaki's PolyBLEP): the correction
     * subtracted/added at a waveform discontinuity so naive saw/square
     * generation doesn't alias. `t` is the oscillator phase (0..1) and `dt`
     * is the phase increment for one sample.
     */
    inline float polyBlep(float t, float dt)
    {
        if (t < dt)
        {
            t = t / dt;
            return t + t - t * t - 1.0f;
        }
        if (t > 1.0f - dt)
        {
            t = (t - 1.0f) / dt;
            return t * t + t + t + 1.0f;
        }
        return 0.0f;
    }

    /** Band-limited sawtooth, falling from +1 to -1 across one cycle. */
    inline float polyBlepSaw(float phase, float phaseIncrement)
    {
        const float naive = 2.0f * phase - 1.0f;
        return naive - polyBlep(phase, phaseIncrement);
    }

    /** Band-limited pulse wave; `pulseWidth` in (0, 1) sets the duty cycle. */
    inline float polyBlepSquare(float phase, float phaseIncrement, float pulseWidth)
    {
        float value = phase < pulseWidth ? 1.0f : -1.0f;
        value += polyBlep(phase, phaseIncrement);

        float shiftedPhase = phase - pulseWidth;
        if (shiftedPhase < 0.0f)
            shiftedPhase += 1.0f;
        value -= polyBlep(shiftedPhase, phaseIncrement);

        return value;
    }
}
