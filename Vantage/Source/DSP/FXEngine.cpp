#include "FXEngine.h"
#include <cmath>

namespace vantage
{
    namespace
    {
        constexpr float kMaxDelaySeconds = 2.5f;
        constexpr float kWowDepthMs = 8.0f;
        constexpr float kFlutterDepthMs = 2.0f;

        float onePoleCoeff(float cutoffHz, double sampleRate)
        {
            return 1.0f - std::exp(-2.0f * juce::MathConstants<float>::pi * cutoffHz / static_cast<float>(sampleRate));
        }
    }

    void FXEngine::prepare(double newSampleRate, int samplesPerBlock, int numChannels)
    {
        sampleRate = newSampleRate;

        delayLine.setMaximumDelayInSamples(static_cast<int>(kMaxDelaySeconds * sampleRate) + 1);
        delayLine.prepare({ sampleRate, static_cast<juce::uint32>(samplesPerBlock), static_cast<juce::uint32>(numChannels) });

        reverb.prepare({ sampleRate, static_cast<juce::uint32>(samplesPerBlock), static_cast<juce::uint32>(numChannels) });

        reset();

        setDelayParameters(delayParams);
        setReverbParameters(reverbParams);
    }

    void FXEngine::reset()
    {
        delayLine.reset();
        reverb.reset();
        delayToneState.fill(0.0f);
        wowPhase = 0.0f;
        flutterPhase = 0.0f;
    }

    void FXEngine::setDelayParameters(const DelayParams& newParams)
    {
        delayParams = newParams;
        delayToneCoeff = onePoleCoeff(delayParams.toneHz, sampleRate);
    }

    void FXEngine::setReverbParameters(const ReverbParams& newParams)
    {
        reverbParams = newParams;

        juce::dsp::Reverb::Parameters reverbDspParams;
        reverbDspParams.roomSize = juce::jlimit(0.0f, 1.0f, reverbParams.size01);
        reverbDspParams.damping = juce::jlimit(0.0f, 1.0f, 1.0f - reverbParams.diffusion01);
        reverbDspParams.wetLevel = juce::jlimit(0.0f, 1.0f, reverbParams.mix01);
        reverbDspParams.dryLevel = juce::jlimit(0.0f, 1.0f, 1.0f - reverbParams.mix01);
        reverbDspParams.width = 1.0f;
        reverbDspParams.freezeMode = 0.0f;
        reverb.setParameters(reverbDspParams);
    }

    void FXEngine::process(juce::AudioBuffer<float>& buffer)
    {
        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();
        const float maxDelaySamples = static_cast<float>(delayLine.getMaximumDelayInSamples());

        const float wowIncrement = static_cast<float>(delayParams.wowRateHz / sampleRate);
        const float flutterIncrement = static_cast<float>(delayParams.flutterRateHz / sampleRate);

        for (int n = 0; n < numSamples; ++n)
        {
            wowPhase += wowIncrement;
            if (wowPhase >= 1.0f)
                wowPhase -= 1.0f;

            flutterPhase += flutterIncrement;
            if (flutterPhase >= 1.0f)
                flutterPhase -= 1.0f;

            const float wowOffsetMs = std::sin(juce::MathConstants<float>::twoPi * wowPhase) * delayParams.wowDepth01 * kWowDepthMs;
            const float flutterOffsetMs = std::sin(juce::MathConstants<float>::twoPi * flutterPhase) * delayParams.flutterDepth01 * kFlutterDepthMs;
            const float delayMs = juce::jmax(1.0f, delayParams.timeMs + wowOffsetMs + flutterOffsetMs);
            const float delaySamples = juce::jlimit(1.0f, maxDelaySamples, delayMs * 0.001f * static_cast<float>(sampleRate));
            delayLine.setDelay(delaySamples);

            for (int ch = 0; ch < numChannels; ++ch)
            {
                const float dry = buffer.getSample(ch, n);
                const float wet = delayLine.popSample(ch);

                auto& toneState = delayToneState[static_cast<size_t>(ch % 2)];
                toneState += delayToneCoeff * (wet - toneState);

                delayLine.pushSample(ch, dry + toneState * delayParams.feedback01);
                buffer.setSample(ch, n, dry * (1.0f - delayParams.mix01) + toneState * delayParams.mix01);
            }
        }

        juce::dsp::AudioBlock<float> block(buffer);
        juce::dsp::ProcessContextReplacing<float> context(block);
        reverb.process(context);
    }
}
