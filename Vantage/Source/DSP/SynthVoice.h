#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "OscillatorEngine.h"
#include "FilterEngine.h"
#include "EnvelopeEngine.h"
#include "LFOEngine.h"
#include "NoiseGenerator.h"
#include "ModulationMatrix.h"
#include "VoiceParameters.h"

namespace vantage
{
    // Envelope slot indices. ENV 1 and ENV 2 are hardwired; ENV 3 is a free
    // modulation source routed only through the matrix.
    constexpr int kAmpEnvIndex = 0;
    constexpr int kFilterEnvIndex = 1;
    constexpr int kFreeEnvIndex = 2;

    /**
     * One polyphonic voice: 3 oscillators + noise, routed into 2 filters
     * (series or parallel), shaped by 3 envelopes, with its own 6 LFOs and
     * modulation matrix so every parameter can be modulated per-voice.
     */
    class SynthVoice : public juce::SynthesiserVoice
    {
    public:
        /** `sharedParams` is owned by the processor and refreshed from the APVTS once per block. */
        explicit SynthVoice(const VoiceParameters& sharedParams) : params(sharedParams) {}

        void prepare(double newSampleRate);

        using juce::SynthesiserVoice::renderNextBlock;

        bool canPlaySound(juce::SynthesiserSound* sound) override;
        void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
        void stopNote(float velocity, bool allowTailOff) override;
        void pitchWheelMoved(int newPitchWheelValue) override;
        void controllerMoved(int controllerNumber, int newControllerValue) override;
        void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

    private:
        void applyStaticParameters();
        void renderOneSample(float& outLeft, float& outRight);

        std::array<OscillatorEngine, kNumOscillators> oscillators;
        NoiseGenerator noise;
        std::array<FilterEngine, kNumFilters> filters;
        std::array<EnvelopeEngine, kNumEnvelopes> envelopes;
        std::array<LFOEngine, kNumLFOs> lfos;
        ModulationMatrix modMatrix;

        const VoiceParameters& params;
        double sampleRate = 44100.0;
        float baseFrequencyHz = 440.0f;
        float velocityGain = 1.0f;
        float pitchWheelSemitones = 0.0f;
    };
}
