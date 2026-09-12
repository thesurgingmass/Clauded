#include "FilterEngine.h"
#include <cmath>

namespace vantage
{
    namespace
    {
        constexpr float kPi = 3.14159265358979323846f;

        float asymmetricSoftClip(float x)
        {
            return x >= 0.0f ? std::tanh(x) : 0.7f * std::tanh(x / 0.7f);
        }
    }

    void FilterEngine::prepare(double newSampleRate)
    {
        sampleRate = newSampleRate;
        reset();
    }

    void FilterEngine::reset()
    {
        for (auto& channelStages : ladderStages)
            channelStages.fill(0.0f);
        svfIc1.fill(0.0f);
        svfIc2.fill(0.0f);
        svfIc1B.fill(0.0f);
        svfIc2B.fill(0.0f);
    }

    void FilterEngine::setCutoffHz(float newCutoffHz)
    {
        cutoffHz = juce::jlimit(20.0f, 20000.0f, newCutoffHz);
        ladderG = 1.0f - std::exp(-2.0f * kPi * cutoffHz / static_cast<float>(sampleRate));
        svfG = std::tan(kPi * cutoffHz / static_cast<float>(sampleRate));
    }

    void FilterEngine::setResonance(float newResonance01)
    {
        resonance01 = juce::jlimit(0.0f, 1.0f, newResonance01);

        ladderFeedback = model == FilterModel::Roland
            ? std::pow(resonance01, 1.5f) * 3.5f
            : resonance01 * 4.0f;

        const float maxQ = model == FilterModel::Nord ? 20.0f : 15.0f;
        svfK = 1.0f / juce::jmap(resonance01, 0.0f, 1.0f, 0.5f, maxQ);
    }

    float FilterEngine::processSvfLowpass(float input, float g, float k, float& ic1, float& ic2) const
    {
        const float a1 = 1.0f / (1.0f + g * (g + k));
        const float a2 = g * a1;
        const float a3 = g * a2;

        const float v3 = input - ic2;
        const float v1 = a1 * ic1 + a2 * v3;
        const float v2 = ic2 + a2 * ic1 + a3 * v3;

        ic1 = 2.0f * v1 - ic1;
        ic2 = 2.0f * v2 - ic2;

        return v2;
    }

    float FilterEngine::processMoog(int channel, float input)
    {
        auto& stages = ladderStages[static_cast<size_t>(channel)];

        const float saturatedInput = std::tanh(input - stages[3] * ladderFeedback);
        stages[0] += ladderG * (saturatedInput - stages[0]);
        stages[1] += ladderG * (std::tanh(stages[0]) - stages[1]);
        stages[2] += ladderG * (std::tanh(stages[1]) - stages[2]);
        stages[3] += ladderG * (std::tanh(stages[2]) - stages[3]);

        return stages[3];
    }

    float FilterEngine::processRoland(int channel, float input)
    {
        auto& stages = ladderStages[static_cast<size_t>(channel)];

        const float saturatedInput = asymmetricSoftClip(input - stages[3] * ladderFeedback);
        stages[0] += ladderG * (saturatedInput - stages[0]);
        stages[1] += ladderG * (asymmetricSoftClip(stages[0]) - stages[1]);
        stages[2] += ladderG * (asymmetricSoftClip(stages[1]) - stages[2]);
        stages[3] += ladderG * (asymmetricSoftClip(stages[2]) - stages[3]);

        return stages[3];
    }

    float FilterEngine::processOberheim(int channel, float input)
    {
        const float lowpass = processSvfLowpass(input, svfG, svfK, svfIc1[static_cast<size_t>(channel)], svfIc2[static_cast<size_t>(channel)]);
        return std::tanh(lowpass * (1.0f + drive * 3.0f));
    }

    float FilterEngine::processNord(int channel, float input)
    {
        const auto ch = static_cast<size_t>(channel);
        const float stageA = processSvfLowpass(input, svfG, svfK, svfIc1[ch], svfIc2[ch]);
        const float stageB = processSvfLowpass(stageA, svfG, svfK, svfIc1B[ch], svfIc2B[ch]);
        return drive > 0.0f ? juce::jlimit(-1.0f, 1.0f, stageB * (1.0f + drive * 3.0f)) : stageB;
    }

    float FilterEngine::processSample(int channel, float input)
    {
        switch (model)
        {
            case FilterModel::Moog:     return processMoog(channel, input);
            case FilterModel::Oberheim: return processOberheim(channel, input);
            case FilterModel::Roland:   return processRoland(channel, input);
            case FilterModel::Nord:     return processNord(channel, input);
            case FilterModel::count:
            default:                    return processOberheim(channel, input);
        }
    }
}
