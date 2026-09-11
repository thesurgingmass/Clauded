#include "Lfo.h"
#include "DspUtils.h"
#include <cmath>
#include <algorithm>

namespace fs
{
    void Lfo::reset()
    {
        phase = 0.0;
        previousPhase = 0.0;
        elapsedSeconds = 0.0;
        heldValue = 0.0f;
    }

    float Lfo::getNextSample()
    {
        previousPhase = phase;
        phase += static_cast<double>(rateHz) / sampleRate;
        if (phase >= 1.0)
            phase -= 1.0;

        float raw = 0.0f;
        switch (shape)
        {
            case LfoShape::sine:
                raw = std::sin(twoPi * static_cast<float>(phase));
                break;
            case LfoShape::triangle:
                raw = phase < 0.5 ? static_cast<float>(4.0 * phase - 1.0)
                                  : static_cast<float>(3.0 - 4.0 * phase);
                break;
            case LfoShape::saw:
                raw = static_cast<float>(2.0 * phase - 1.0);
                break;
            case LfoShape::square:
                raw = phase < 0.5 ? 1.0f : -1.0f;
                break;
            case LfoShape::sampleAndHold:
                if (phase < previousPhase)
                    heldValue = distribution(rng);
                raw = heldValue;
                break;
        }

        elapsedSeconds += 1.0 / sampleRate;
        const float fade = delaySeconds > 0.0f
            ? clamp01(static_cast<float>((elapsedSeconds - delaySeconds) / fadeInSeconds))
            : 1.0f;

        return raw * fade;
    }
}
