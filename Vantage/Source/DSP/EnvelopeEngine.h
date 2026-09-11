#pragma once

#include <juce_dsp/juce_dsp.h>

namespace vantage
{
    /**
     * ADSR envelope wrapper. Runs a standard linear/exponential ADSR for now;
     * the SH-101-style snappy attack/decay curve shaping is added on top of
     * this same interface in a later phase.
     */
    class EnvelopeEngine
    {
    public:
        void prepare(double newSampleRate);
        void reset();

        void setParameters(float attackSeconds, float decaySeconds, float sustain01, float releaseSeconds);

        void noteOn();
        void noteOff();
        bool isActive() const { return adsr.isActive(); }

        float renderSample();

    private:
        juce::ADSR adsr;
    };
}
