#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "DSP/Constants.h"
#include "DSP/VoiceParameters.h"
#include "DSP/FXEngine.h"

namespace vantage
{
    namespace ParamIDs
    {
        juce::String oscModel(int oscIndex);
        juce::String oscLevel(int oscIndex);
        juce::String oscCoarse(int oscIndex);
        juce::String oscFine(int oscIndex);
        juce::String oscPan(int oscIndex);
        juce::String oscWavetablePosition(int oscIndex);
        juce::String oscRouting(int oscIndex);

        juce::String noiseColor();
        juce::String noiseLevel();

        juce::String filterModel(int filterIndex);
        juce::String filterCutoff(int filterIndex);
        juce::String filterResonance(int filterIndex);
        juce::String filterDrive(int filterIndex);
        juce::String filterRouting();

        juce::String envAttack(int envIndex);
        juce::String envDecay(int envIndex);
        juce::String envSustain(int envIndex);
        juce::String envRelease(int envIndex);
        juce::String filterEnvAmount();

        juce::String lfoRate(int lfoIndex);
        juce::String lfoWaveform(int lfoIndex);

        juce::String modSource(int slotIndex);
        juce::String modDestination(int slotIndex);
        juce::String modDepth(int slotIndex);

        juce::String delayTime();
        juce::String delayFeedback();
        juce::String delayWowRate();
        juce::String delayWowDepth();
        juce::String delayFlutterRate();
        juce::String delayFlutterDepth();
        juce::String delayTone();
        juce::String delayMix();

        juce::String reverbSize();
        juce::String reverbDecay();
        juce::String reverbMix();
        juce::String reverbDiffusion();
        juce::String reverbModDepth();
        juce::String reverbModRate();
        juce::String reverbLowCut();
        juce::String reverbHighCut();

        juce::String masterGain();
        juce::String polyphonyMode();
        juce::String glideTimeMs();
        juce::String legato();
    }

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    /**
     * Parameters shared by the whole processor rather than owned per-voice.
     * `glideTimeMs` (per-voice portamento) lives on VoiceParameters instead,
     * since that's what SynthVoice itself reads each block.
     */
    struct GlobalParameters
    {
        FXEngine::DelayParams delay;
        FXEngine::ReverbParams reverb;
        float masterGain = 0.8f;
        PolyphonyMode polyphonyMode = PolyphonyMode::Poly;
        bool legato = false;
    };

    /** Reads every current parameter value out of the APVTS into a VoiceParameters. */
    void updateVoiceParameters(VoiceParameters& voiceParams, const juce::AudioProcessorValueTreeState& apvts);

    /** Reads every current parameter value out of the APVTS into a GlobalParameters. */
    void updateGlobalParameters(GlobalParameters& globalParams, const juce::AudioProcessorValueTreeState& apvts);
}
