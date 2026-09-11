#pragma once

#include <juce_core/juce_core.h>
#include "Constants.h"

namespace vantage
{
    /** Free-running LFO with a selectable waveform, output range [-1, 1]. */
    class LFOEngine
    {
    public:
        void prepare(double newSampleRate);
        void reset();

        void setRateHz(float newRateHz) { rateHz = juce::jmax(0.001f, newRateHz); }
        void setWaveform(LFOWaveform newWaveform) { waveform = newWaveform; }

        float renderSample();

    private:
        double sampleRate = 44100.0;
        float phase = 0.0f;
        float rateHz = 1.0f;
        LFOWaveform waveform = LFOWaveform::Sine;

        juce::Random random;
        float lastRandomValue = 0.0f;
    };
}
