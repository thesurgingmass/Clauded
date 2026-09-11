#include "OscillatorEngine.h"
#include <cmath>

namespace vantage
{
    void OscillatorEngine::prepare(double newSampleRate)
    {
        sampleRate = newSampleRate;
        reset();
    }

    void OscillatorEngine::reset()
    {
        phase = 0.0f;
    }

    void OscillatorEngine::setFrequency(float frequencyHz)
    {
        phaseIncrement = static_cast<float>(frequencyHz / sampleRate);
    }

    float OscillatorEngine::renderSample()
    {
        const float sample = std::sin(juce::MathConstants<float>::twoPi * phase);

        phase += phaseIncrement;
        if (phase >= 1.0f)
            phase -= 1.0f;

        return sample;
    }
}
