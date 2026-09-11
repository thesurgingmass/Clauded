#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "FMAlgorithms.h"
#include "Common/FMOperator.h"
#include "Common/FormantGenerator.h"
#include "Common/NoiseSource.h"
#include "Common/MultiModeFilter.h"
#include "Common/Lfo.h"
#include "Common/Drive.h"
#include "Common/RateLevelEnvelope.h"

namespace formantone
{
    /** Per-operator settings, refreshed from the APVTS once per block. */
    struct OperatorSettings
    {
        double ratioCoarse = 1.0;
        double ratioFine = 0.0;
        bool fixedFrequencyMode = false;
        double fixedFrequencyHz = 440.0;
        float level = 1.0f;
        float feedback = 0.0f; // only applied to operator 0
        fs::RateLevelEnvelope::Parameters envelope;
    };

    /** Per-formant-generator settings, refreshed from the APVTS once per block. */
    struct FormantSettings
    {
        float frequencyHz = 800.0f;
        float bandwidth = 0.2f;
        float level = 0.5f;
        fs::RateLevelEnvelope::Parameters envelope;
    };

    struct LfoSettings
    {
        fs::LfoShape shape = fs::LfoShape::triangle;
        float rateHz = 4.0f;
        float delaySeconds = 0.0f;
        float pitchDepthSemitones = 0.0f;
        float ampDepth = 0.0f;       // 0..1, applied as +/-depth/2 tremolo around unity
        float filterDepthOctaves = 0.0f;
    };

    /** Everything a voice needs to render; owned and refreshed by the processor. */
    struct VoiceParameters
    {
        int algorithmIndex = 0;
        std::array<OperatorSettings, numOperators> operators;

        std::array<FormantSettings, 3> formants;
        float voicedUnvoiced = 0.0f;   // 0 = fully voiced, 1 = fully unvoiced
        float fmFormantBalance = 0.3f; // 0 = pure FM engine, 1 = pure formant engine

        fs::RateLevelEnvelope::Parameters ampEnvelope;

        fs::FilterType filterType = fs::FilterType::lpf24;
        float filterCutoffHz = 8000.0f;
        float filterResonance = 0.707f;
        float filterEnvAmount = 0.0f;
        float filterKeyTrack = 0.0f;
        fs::RateLevelEnvelope::Parameters filterEnvelope;

        LfoSettings lfo;

        float driveAmount = 0.0f;
        float outputGain = 1.0f;
    };

    class FormantOneVoice : public juce::SynthesiserVoice
    {
    public:
        explicit FormantOneVoice(const VoiceParameters& paramsRef) : params(paramsRef) {}

        using juce::SynthesiserVoice::renderNextBlock;

        bool canPlaySound(juce::SynthesiserSound*) override { return true; }

        void prepare(double sampleRate);

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

        std::array<fs::FMOperator, numOperators> operators;
        std::array<fs::FormantGenerator, 3> formants;
        fs::NoiseSource noise;
        fs::RateLevelEnvelope ampEnvelope;
        fs::MultiModeFilter filter;
        fs::Lfo lfo;
        fs::Drive drive;

        double sampleRate = 44100.0;
        float currentVelocity = 1.0f;
        float baseNote = 60.0f;
        float pitchWheelSemitoneOffset = 0.0f;
        static constexpr float pitchBendRangeSemitones = 2.0f;

        bool active = false;
    };
}
