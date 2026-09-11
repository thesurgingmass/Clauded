#pragma once

#include <juce_dsp/juce_dsp.h>
#include "Constants.h"

namespace vantage
{
    /**
     * One filter slot. `model` selects which manufacturer's topology will be
     * modeled once the per-model DSP is implemented; for now every model runs
     * the same state-variable lowpass so the voice graph compiles and passes
     * audio end to end.
     */
    class FilterEngine
    {
    public:
        void prepare(double newSampleRate);
        void reset();

        void setModel(FilterModel newModel) { model = newModel; }
        FilterModel getModel() const { return model; }

        void setCutoffHz(float cutoffHz);
        void setResonance(float resonance01);
        void setDrive(float drive01) { drive = juce::jlimit(0.0f, 1.0f, drive01); }

        float processSample(int channel, float input);

    private:
        FilterModel model = FilterModel::Moog;
        juce::dsp::StateVariableTPTFilter<float> filter;
        float drive = 0.0f;
    };
}
