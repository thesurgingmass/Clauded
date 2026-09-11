#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "MachineParamSpec.h"
#include "MachineEngine.h"
#include "dsp/Filter.h"
#include "dsp/Envelope.h"
#include <memory>
#include <vector>
#include <atomic>

namespace md
{
    /** Shared AudioProcessor for every Machinedrum-inspired instrument.
        Owns the machine-agnostic MIDI-trigger logic, the shared FLT/AMP
        signal chain, the keyboard-play toggle, and state save/restore.
        Each machine plugin only supplies a MachineInfo (its SYN knob
        layout) and a MachineEngine (its SYN-stage synthesis). */
    class MachineProcessor : public juce::AudioProcessor
    {
    private:
        // Declared first so they are constructed before apvts below, which
        // depends on machineInfo when building its parameter layout.
        MachineInfo machineInfo;
        std::unique_ptr<MachineEngine> engine;

    public:
        MachineProcessor(MachineInfo infoIn, std::unique_ptr<MachineEngine> engineIn);
        ~MachineProcessor() override = default;

        void prepareToPlay(double sampleRate, int samplesPerBlock) override;
        void releaseResources() override {}
        bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
        void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

        juce::AudioProcessorEditor* createEditor() override;
        bool hasEditor() const override { return true; }

        const juce::String getName() const override { return machineInfo.machineName; }
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

        const MachineInfo& getMachineInfo() const { return machineInfo; }

        juce::AudioProcessorValueTreeState apvts;

        static constexpr int rootNote = 60; // C3: keyboard-mode pitch-tracking center

    private:
        juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
        void renderRange(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
        void handleNoteOn(int noteNumber, float velocity01);

        StateVariableFilter filter;
        AHDEnvelope filterEnv;
        AHDEnvelope ampEnv;

        std::vector<std::atomic<float>*> synParamPtrs;
        std::atomic<float>* fltFreq = nullptr;
        std::atomic<float>* fltRes  = nullptr;
        std::atomic<float>* fltAtk  = nullptr;
        std::atomic<float>* fltDec  = nullptr;
        std::atomic<float>* ampAtk  = nullptr;
        std::atomic<float>* ampHold = nullptr;
        std::atomic<float>* ampDec  = nullptr;
        std::atomic<float>* ampVol  = nullptr;
        std::atomic<float>* ampOd   = nullptr;
        std::atomic<float>* kybdMode = nullptr;

        std::vector<float> synValuesCache;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MachineProcessor)
    };
}
