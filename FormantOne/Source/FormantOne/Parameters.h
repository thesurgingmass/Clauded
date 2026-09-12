#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include "FMAlgorithms.h"
#include "FormantOneVoice.h"

namespace formantone
{
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

    /**
        Every operator/formant/amp/filter envelope in this plugin is the
        same 4-stage rate/level shape (see RateLevelEnvelope), exposed to
        the user as 7 controls: Attack, Level 1, Decay 1, Level 2, Decay 2,
        Sustain (= level 3, held while the note is down), Release. This
        table drives both parameter creation and the generic editor
        controls for every envelope "owner" (op1..op8, formant1..3, amp,
        filterEnv) so that shape only has to be described once.
    */
    struct EnvelopeStage
    {
        const char* idSuffix;
        const char* label;      // full name, used in the automation-visible parameter name
        const char* shortLabel; // compact caption for a dense fader-strip GUI
        bool isTimeParam;       // true = ms time control, false = 0..1 level control
    };

    inline const std::array<EnvelopeStage, 7>& getEnvelopeStages()
    {
        static const std::array<EnvelopeStage, 7> stages { {
            { "attack", "Attack", "Atk", true },
            { "level1", "Level 1", "L1", false },
            { "decay1", "Decay 1", "Dec1", true },
            { "level2", "Level 2", "L2", false },
            { "decay2", "Decay 2", "Dec2", true },
            { "sustain", "Sustain", "Sus", false },
            { "release", "Release", "Rel", true },
        } };
        return stages;
    }

    namespace ParamIDs
    {
        juce::String algorithm();

        juce::String opRatioCoarse(int opIndex);
        juce::String opRatioFine(int opIndex);
        juce::String opFixedMode(int opIndex);
        juce::String opFixedFreq(int opIndex);
        juce::String opLevel(int opIndex);
        juce::String opFeedback(); // operator 1 only
        juce::String envelopeId(const juce::String& owner, const char* stageSuffix);
        juce::String opOwner(int opIndex);

        juce::String formantFrequency(int formantIndex);
        juce::String formantBandwidth(int formantIndex);
        juce::String formantLevel(int formantIndex);
        juce::String formantOwner(int formantIndex);

        juce::String voicedUnvoiced();
        juce::String fmFormantBalance();

        juce::String ampOwner();

        juce::String filterType();
        juce::String filterCutoff();
        juce::String filterResonance();
        juce::String filterEnvAmount();
        juce::String filterKeyTrack();
        juce::String filterEnvOwner();

        juce::String lfoShape();
        juce::String lfoRate();
        juce::String lfoDelay();
        juce::String lfoPitchDepth();
        juce::String lfoAmpDepth();
        juce::String lfoFilterDepth();

        juce::String driveAmount();
        juce::String outputGain();
    }

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    /** Reads every current parameter value out of the APVTS into a VoiceParameters. */
    void updateVoiceParameters(VoiceParameters& voiceParams, const juce::AudioProcessorValueTreeState& apvts);

    /** Reads one envelope "owner"'s 7 stage parameters into a RateLevelEnvelope::Parameters. */
    fs::RateLevelEnvelope::Parameters readEnvelopeParameters(const juce::AudioProcessorValueTreeState& apvts,
                                                              const juce::String& owner);
}
