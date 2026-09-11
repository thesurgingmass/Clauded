#pragma once

#include <juce_core/juce_core.h>
#include "Constants.h"

namespace vantage
{
    /**
     * One oscillator slot. `model` selects which analog/digital character will
     * shape its waveform once the per-model DSP is implemented; for now every
     * model renders the same band-unlimited sine so the voice graph compiles
     * and passes audio end to end.
     */
    class OscillatorEngine
    {
    public:
        void prepare(double newSampleRate);
        void reset();

        void setModel(OscillatorModel newModel) { model = newModel; }
        OscillatorModel getModel() const { return model; }

        void setFrequency(float frequencyHz);
        void setWavetablePosition(float position01) { wavetablePosition = juce::jlimit(0.0f, 1.0f, position01); }

        float renderSample();

    private:
        OscillatorModel model = OscillatorModel::Moog;
        double sampleRate = 44100.0;
        float phase = 0.0f;
        float phaseIncrement = 0.0f;
        float wavetablePosition = 0.0f;
    };
}
