#include "FormantGenerator.h"
#include "DspUtils.h"
#include <cmath>

namespace fs
{
    void FormantGenerator::prepare(double newSampleRate)
    {
        sampleRate = newSampleRate;
        envelope.setSampleRate(newSampleRate);
        juce::dsp::ProcessSpec spec { newSampleRate, 1, 1 };
        noiseFilter.prepare(spec);
        reset();
    }

    void FormantGenerator::reset()
    {
        periodPhase = 0.0;
        formantPhase = 0.0;
        envelope.reset();
        noiseFilter.reset();
    }

    void FormantGenerator::noteOn()
    {
        envelope.noteOn();
    }

    void FormantGenerator::noteOff()
    {
        envelope.noteOff();
    }

    void FormantGenerator::setBandwidth(float widthFraction01)
    {
        bandwidth = juce::jlimit(0.02f, 1.0f, widthFraction01);
    }

    void FormantGenerator::setVoicedUnvoiced(float amount01)
    {
        voicedUnvoiced = clamp01(amount01);
    }

    void FormantGenerator::updateForBlock()
    {
        const float clampedFrequency = juce::jlimit(20.0f, static_cast<float>(sampleRate) * 0.49f, frequencyHz);
        const float q = juce::jlimit(0.5f, 20.0f, 1.0f / bandwidth);
        *noiseFilter.coefficients = *juce::dsp::IIR::Coefficients<float>::makeBandPass(sampleRate, clampedFrequency, q);
    }

    float FormantGenerator::renderSample(double fundamentalHz, float noiseSample)
    {
        periodPhase += fundamentalHz / sampleRate;
        if (periodPhase >= 1.0)
            periodPhase -= 1.0;

        const float window = periodPhase < bandwidth
            ? 0.5f * (1.0f - std::cos(twoPi * static_cast<float>(periodPhase) / bandwidth))
            : 0.0f;

        formantPhase += static_cast<double>(frequencyHz) / sampleRate;
        if (formantPhase >= 1.0)
            formantPhase -= 1.0;

        const float voiced = std::sin(twoPi * static_cast<float>(formantPhase)) * window;
        const float unvoiced = noiseFilter.processSample(noiseSample);

        const float mixed = voiced * (1.0f - voicedUnvoiced) + unvoiced * voicedUnvoiced;
        return mixed * level * envelope.getNextSample();
    }
}
