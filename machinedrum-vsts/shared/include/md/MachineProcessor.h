#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "MachineParamSpec.h"
#include "MachineEngine.h"
#include "dsp/Effects.h"
#include <memory>
#include <vector>
#include <atomic>

namespace md
{
    /** Shared AudioProcessor for every Machinedrum-inspired instrument.

        There is no dedicated "AMP" page on the real hardware -- each
        machine's own SYN parameters already shape its amplitude envelope
        (e.g. EFM-BD's own DEC). What IS shared across every track is:

          - TFX (Track Effects): AMD/AMF (tremolo), EQF/EQG (1-band
            parametric EQ), FLTF/FLTW/FLTQ (a lowpass+highpass pair sharing
            a base cutoff and a gap width), SRR (sample-rate reduction).
          - ROUTING: DIST (overload distortion), VOL, PAN, and DEL/REV
            sends -- LFOS/LFOD/LFOM live on this page too but are the
            excluded LFO-page functionality, so they are omitted here.
          - LEV: the hardware's physical, always-present track level knob,
            separate from the page-based VOL.

        DEL/REV feed a small built-in delay and reverb per plugin instance,
        standing in for the hardware's shared Rhythm Echo / Gate Box buses
        that have no equivalent inside a standalone VST3 instrument.

        Each machine plugin only supplies a MachineInfo (its SYN knob
        layout) and a MachineEngine (its SYN-stage synthesis, envelope
        included). */
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

        // TFX stage
        Tremolo tremolo;
        GapFilter gapFilter;
        juce::dsp::IIR::Filter<float> eq;
        SampleRateReducer srr;
        float lastEqFreq = -1.0f, lastEqGain = -1.0f;

        // ROUTING stage
        SimpleDelay delayL, delayR;
        juce::dsp::Reverb reverb;
        juce::AudioBuffer<float> reverbScratch; // pre-sized in prepareToPlay; never (re)allocated on the audio thread

        std::vector<std::atomic<float>*> synParamPtrs;
        std::atomic<float>* amd = nullptr;
        std::atomic<float>* amf = nullptr;
        std::atomic<float>* eqf = nullptr;
        std::atomic<float>* eqg = nullptr;
        std::atomic<float>* fltf = nullptr;
        std::atomic<float>* fltw = nullptr;
        std::atomic<float>* fltq = nullptr;
        std::atomic<float>* srrParam = nullptr;
        std::atomic<float>* dist = nullptr;
        std::atomic<float>* vol = nullptr;
        std::atomic<float>* pan = nullptr;
        std::atomic<float>* del = nullptr;
        std::atomic<float>* rev = nullptr;
        std::atomic<float>* lev = nullptr;
        std::atomic<float>* kybdMode = nullptr;

        std::vector<float> synValuesCache;
        double currentSampleRate = 44100.0;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MachineProcessor)
    };
}
