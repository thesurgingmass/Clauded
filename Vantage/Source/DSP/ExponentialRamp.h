#pragma once

#include <cmath>

namespace vantage
{
    /**
     * One-pole smoothing coefficient that reaches ~99.9% of a step target
     * within `timeSeconds`: `value += (1 - coeff) * (target - value)` per
     * sample. Returns 0 (instant, snap-to-target) for near-zero times.
     * Shared by EnvelopeEngine's segments and SynthVoice's glide/portamento.
     */
    inline float exponentialRampCoefficient(float timeSeconds, double sampleRate)
    {
        if (timeSeconds <= 0.0005f)
            return 0.0f;

        constexpr float kReach = 6.9077553f; // -ln(1 - 0.999)
        return std::exp(-kReach / (timeSeconds * static_cast<float>(sampleRate)));
    }
}
