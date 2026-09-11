#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "FormantOneVoice.h"
#include "FormantOneSound.h"
#include "Parameters.h"

namespace formantone
{
    constexpr int numVoices = 8; // matches the nominal 8-voice polyphony of the hardware this is inspired by

    class FormantOneAudioProcessor : public juce::AudioProcessor
    {
    public:
        FormantOneAudioProcessor();
        ~FormantOneAudioProcessor() override = default;

        using AudioProcessor::processBlock;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override {}
        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override { return true; }

        const juce::String getName() const override { return "Formant One"; }
        bool acceptsMidi() const override { return true; }
        bool producesMidi() const override { return false; }
        bool isMidiEffect() const override { return false; }
        double getTailLengthSeconds() const override { return 2.0; }

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

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormantOneAudioProcessor)
    };
}
