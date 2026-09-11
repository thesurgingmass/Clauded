#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "FMDrumVoice.h"
#include "FMDrumSound.h"
#include "Parameters.h"

namespace fmdrum
{
    // Monophonic: a single voice, so a new hit immediately cuts off and
    // retriggers whatever's still sounding (juce::Synthesiser's default note
    // stealing does this for free - stopVoice(..., allowTailOff=false) then
    // startVoice on the same voice), matching a drum machine's behavior of
    // notes never overlapping/layering on the same drum.
    constexpr int numVoices = 1;

    class FMDrumAudioProcessor : public juce::AudioProcessor
    {
    public:
        FMDrumAudioProcessor();
        ~FMDrumAudioProcessor() override = default;

        using AudioProcessor::processBlock;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override {}
        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override { return true; }

        const juce::String getName() const override { return "FM Drum"; }
        bool acceptsMidi() const override { return true; }
        bool producesMidi() const override { return false; }
        bool isMidiEffect() const override { return false; }
        double getTailLengthSeconds() const override { return 1.0; }

        int getNumPrograms() override { return 1; }
        int getCurrentProgram() override { return 0; }
        void setCurrentProgram(int) override {}
        const juce::String getProgramName(int) override { return {}; }
        void changeProgramName(int, const juce::String&) override {}

        void getStateInformation(juce::MemoryBlock& destData) override;
        void setStateInformation(const void* data, int sizeInBytes) override;

        juce::AudioProcessorValueTreeState apvts;

    private:
        VoiceParameters voiceParameters;
        juce::Synthesiser synth;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FMDrumAudioProcessor)
    };
}
