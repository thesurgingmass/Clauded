#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "FMAlgorithms.h"
#include "FMToneVoice.h"

namespace fmtone
{
    /** Operator names in fixed index order, used both for param IDs and the UI. */
    constexpr const char* operatorNames[numOperators] = { "C", "A", "B", "D" };

    /** Common FM ratios, spanning the classic DX-style fractional/integer set. */
    inline juce::StringArray getRatioChoices()
    {
        return { "0.25", "0.5", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "14", "16" };
    }

    inline double ratioChoiceToValue(int choiceIndex)
    {
        static const std::array<double, 16> values {
            0.25, 0.5, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 14.0, 16.0
        };
        return values[static_cast<size_t>(juce::jlimit(0, 15, choiceIndex))];
    }

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    /** Reads every current parameter value out of the APVTS into a VoiceParameters. */
    void updateVoiceParameters(VoiceParameters& voiceParams, const juce::AudioProcessorValueTreeState& apvts);

    namespace ParamIDs
    {
        juce::String algorithm();
        juce::String opRatioCoarse(int opIndex);
        juce::String opRatioFine(int opIndex);
        juce::String opLevel(int opIndex);
        juce::String opFeedback(); // operator C only
        juce::String opAttack(int opIndex);
        juce::String opDecay(int opIndex);
        juce::String opSustain(int opIndex);
        juce::String opRelease(int opIndex);

        juce::String ampAttack();
        juce::String ampDecay();
        juce::String ampSustain();
        juce::String ampRelease();

        juce::String filterType();
        juce::String filterCutoff();
        juce::String filterResonance();
        juce::String filterEnvAmount();
        juce::String filterKeyTrack();
        juce::String filterAttack();
        juce::String filterDecay();
        juce::String filterSustain();
        juce::String filterRelease();

        juce::String driveAmount();
        juce::String outputGain();
    }
}
