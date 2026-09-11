#include "NoiseGenerator.h"

namespace vantage
{
    void NoiseGenerator::prepare(double newSampleRate)
    {
        sampleRate = newSampleRate;
        reset();
    }

    void NoiseGenerator::reset()
    {
        pinkState.fill(0.0f);
        brownState = 0.0f;
    }

    float NoiseGenerator::nextWhite()
    {
        return random.nextFloat() * 2.0f - 1.0f;
    }

    float NoiseGenerator::nextPink()
    {
        // Paul Kellet's refined pink noise filter.
        const float white = nextWhite();

        pinkState[0] = 0.99886f * pinkState[0] + white * 0.0555179f;
        pinkState[1] = 0.99332f * pinkState[1] + white * 0.0750759f;
        pinkState[2] = 0.96900f * pinkState[2] + white * 0.1538520f;
        pinkState[3] = 0.86650f * pinkState[3] + white * 0.3104856f;
        pinkState[4] = 0.55000f * pinkState[4] + white * 0.5329522f;
        pinkState[5] = -0.7616f * pinkState[5] - white * 0.0168980f;

        const float pink = pinkState[0] + pinkState[1] + pinkState[2] + pinkState[3]
                          + pinkState[4] + pinkState[5] + pinkState[6] + white * 0.5362f;
        pinkState[6] = white * 0.115926f;

        return pink * 0.11f; // roughly normalize to +-1
    }

    float NoiseGenerator::nextBrown()
    {
        // Leaky integrator: the leak prevents unbounded DC drift while still
        // giving the -6dB/octave brown-noise slope.
        const float white = nextWhite();
        brownState = 0.998f * brownState + white * 0.02f;
        return juce::jlimit(-1.0f, 1.0f, brownState * 6.0f);
    }

    float NoiseGenerator::renderSample(float color)
    {
        color = juce::jlimit(0.0f, 1.0f, color);

        if (color <= 0.5f)
        {
            const float t = color * 2.0f;
            return nextWhite() * (1.0f - t) + nextPink() * t;
        }

        const float t = (color - 0.5f) * 2.0f;
        return nextPink() * (1.0f - t) + nextBrown() * t;
    }
}
