#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "FMAlgorithms.h"
#include "Common/FMOperator.h"
#include "Common/MultiModeFilter.h"
#include "Common/Drive.h"

namespace fmtone
{
    /** Per-operator settings, refreshed from the APVTS once per block. */
    struct OperatorSettings
    {
        double ratioCoarse = 1.0;
        double ratioFine = 0.0;
        float level = 1.0f;
        float feedback = 0.0f;
        juce::ADSR::Parameters envelope;
    };

    /** Everything a voice needs to render; owned and refreshed by the processor. */
    struct VoiceParameters
    {
        int algorithmIndex = 0;
        std::array<OperatorSettings, fmtone::numOperators> operators;
        juce::ADSR::Parameters ampEnvelope;

        fm::FilterType filterType = fm::FilterType::lowPass;
        float filterCutoffHz = 8000.0f;
        float filterResonance = 0.707f;
        float filterEnvAmount = 0.0f;
        float filterKeyTrack = 0.0f;
        juce::ADSR::Parameters filterEnvelope;

        float driveAmount = 0.0f;
        float outputGain = 1.0f;
    };

    class FMToneVoice : public juce::SynthesiserVoice
    {
    public:
        explicit FMToneVoice(const VoiceParameters& paramsRef) : params(paramsRef) {}

        using juce::SynthesiserVoice::renderNextBlock;

        bool canPlaySound(juce::SynthesiserSound*) override { return true; }

        void prepare(const juce::dsp::ProcessSpec& spec);

        void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*,
                        int currentPitchWheelPosition) override;
        void stopNote(float velocity, bool allowTailOff) override;
        void pitchWheelMoved(int newPitchWheelValue) override;
        void controllerMoved(int, int) override {}
        void aftertouchChanged(int) override {}
        void channelPressureChanged(int) override {}

        void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

        bool isVoiceActive() const override { return active; }

    private:
        void applyParametersForCurrentNote();

        const VoiceParameters& params;

        std::array<fm::FMOperator, fmtone::numOperators> operators;
        juce::ADSR ampEnvelope;
        fm::MultiModeFilter filter;
        fm::Drive drive;

        double sampleRate = 44100.0;
        float currentVelocity = 1.0f;
        float baseNote = 60.0f;
        float pitchWheelSemitoneOffset = 0.0f;
        static constexpr float pitchBendRangeSemitones = 2.0f;

        bool active = false;
    };
}
