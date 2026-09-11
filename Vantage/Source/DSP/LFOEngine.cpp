#include "LFOEngine.h"
#include <cmath>

namespace vantage
{
    void LFOEngine::prepare(double newSampleRate)
    {
        sampleRate = newSampleRate;
        reset();
    }

    void LFOEngine::reset()
    {
        phase = 0.0f;
        lastRandomValue = 0.0f;
    }

    float LFOEngine::renderSample()
    {
        float value = 0.0f;

        switch (waveform)
        {
            case LFOWaveform::Sine:
                value = std::sin(juce::MathConstants<float>::twoPi * phase);
                break;

            case LFOWaveform::Triangle:
                value = 4.0f * std::abs(phase - std::floor(phase + 0.75f) + 0.25f) - 1.0f;
                break;

            case LFOWaveform::Saw:
                value = 2.0f * phase - 1.0f;
                break;

            case LFOWaveform::Square:
                value = phase < 0.5f ? 1.0f : -1.0f;
                break;

            case LFOWaveform::SampleAndHold:
                value = lastRandomValue;
                break;

            case LFOWaveform::count:
            default:
                break;
        }

        phase += static_cast<float>(rateHz / sampleRate);
        if (phase >= 1.0f)
        {
            phase -= 1.0f;
            if (waveform == LFOWaveform::SampleAndHold)
                lastRandomValue = random.nextFloat() * 2.0f - 1.0f;
        }

        return value;
    }
}
