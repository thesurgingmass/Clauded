#include "FilterEngine.h"
#include <cmath>

namespace vantage
{
    void FilterEngine::prepare(double newSampleRate)
    {
        filter.prepare({ newSampleRate, 512, 2 });
        filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
        reset();
    }

    void FilterEngine::reset()
    {
        filter.reset();
    }

    void FilterEngine::setCutoffHz(float cutoffHz)
    {
        filter.setCutoffFrequency(juce::jlimit(20.0f, 20000.0f, cutoffHz));
    }

    void FilterEngine::setResonance(float resonance01)
    {
        // Map 0-1 UI resonance to JUCE's Q range, biased so 0.5 sits near unity Q.
        filter.setResonance(juce::jmap(juce::jlimit(0.0f, 1.0f, resonance01), 0.0f, 1.0f, 0.5f, 10.0f));
    }

    float FilterEngine::processSample(int channel, float input)
    {
        const float driven = std::tanh(input * (1.0f + drive * 3.0f));
        return filter.processSample(channel, driven);
    }
}
