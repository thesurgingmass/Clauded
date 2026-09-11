#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "FMToneVoice.h"
#include "FMToneSound.h"
#include "Parameters.h"

namespace fmtone
{
    constexpr int numVoices = 16;

    class FMToneAudioProcessor : public juce::AudioProcessor
    {
    public:
        FMToneAudioProcessor();
        ~FMToneAudioProcessor() override = default;

        using AudioProcessor::processBlock;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override {}
        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override { return true; }

        const juce::String getName() const override { return "FM Tone"; }
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

        // Individual voices are already soft-clipped by the drive stage, but
        // summing many simultaneous voices together can still push well past
        // 0dBFS; this is the final safety net so that never turns into hard
        // digital clipping at the host boundary.
        juce::dsp::Limiter<float> outputLimiter;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FMToneAudioProcessor)
    };
}
