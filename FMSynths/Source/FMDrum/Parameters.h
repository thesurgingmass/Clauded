#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "FMDrumVoice.h"

namespace fmdrum
{
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateVoiceParameters(VoiceParameters& voiceParams, const juce::AudioProcessorValueTreeState& apvts);

    namespace ParamIDs
    {
        juce::String bodyRatio();
        juce::String bodyTune();
        juce::String bodyIndex();
        juce::String bodyFeedback();
        juce::String bodyLevel();

        juce::String pitchEnvAmount();
        juce::String pitchEnvDecay();

        juce::String noiseLevel();
        juce::String noiseFilterType();
        juce::String noiseFilterCutoff();
        juce::String noiseAttack();
        juce::String noiseDecay();
        juce::String noiseSustain();
        juce::String noiseRelease();

        juce::String transientLevel();
        juce::String transientDecay();

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
