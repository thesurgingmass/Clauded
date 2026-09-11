#pragma once

#include <juce_core/juce_core.h>
#include <array>

namespace vantage
{
    /**
     * Continuous white -> pink -> brown noise source. `color` sweeps the whole
     * range: 0 = white, 0.5 = pink, 1 = brown, with a linear crossfade between
     * adjacent bands so the morph has no audible stepping.
     */
    class NoiseGenerator
    {
    public:
        void prepare(double newSampleRate);
        void reset();

        float renderSample(float color);

    private:
        float nextWhite();
        float nextPink();
        float nextBrown();

        juce::Random random;
        double sampleRate = 44100.0;

        std::array<float, 7> pinkState {};
        float brownState = 0.0f;
    };
}
