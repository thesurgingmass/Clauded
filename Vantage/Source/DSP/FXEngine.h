#pragma once

#include <juce_dsp/juce_dsp.h>
#include <array>

namespace vantage
{
    /**
     * Send-effects chain: a tape-style delay (with wow/flutter and a
     * feedback-path tone filter) into a spatial reverb. The reverb currently
     * runs juce::dsp::Reverb as a stand-in for the eventual Supermassive-style
     * granular/delay-network algorithm, so `diffusion`/`modDepth`/`modRate`/
     * the low/high cuts are accepted and cached now but not yet applied.
     */
    class FXEngine
    {
    public:
        struct DelayParams
        {
            float timeMs = 350.0f;
            float feedback01 = 0.35f;
            float wowRateHz = 0.8f;
            float wowDepth01 = 0.0f;
            float flutterRateHz = 6.0f;
            float flutterDepth01 = 0.0f;
            float toneHz = 4000.0f;
            float mix01 = 0.0f;
        };

        struct ReverbParams
        {
            float size01 = 0.5f;
            float decay01 = 0.5f;
            float mix01 = 0.0f;
            float diffusion01 = 0.5f;
            float modDepth01 = 0.2f;
            float modRateHz = 0.3f;
            float lowCutHz = 100.0f;
            float highCutHz = 12000.0f;
        };

        void prepare(double newSampleRate, int samplesPerBlock, int numChannels);
        void reset();

        void setDelayParameters(const DelayParams& newParams);
        void setReverbParameters(const ReverbParams& newParams);

        void process(juce::AudioBuffer<float>& buffer);

    private:
        double sampleRate = 44100.0;

        DelayParams delayParams;
        juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Lagrange3rd> delayLine { 1 };
        std::array<float, 2> delayToneState {};
        float delayToneCoeff = 0.5f;
        float wowPhase = 0.0f;
        float flutterPhase = 0.0f;

        ReverbParams reverbParams;
        juce::dsp::Reverb reverb;
    };
}
