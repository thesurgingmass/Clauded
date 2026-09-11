#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Common/FMOperator.h"
#include "Common/MultiModeFilter.h"
#include "Common/Drive.h"

namespace fmdrum
{
    /** Everything a voice needs to render; owned and refreshed by the processor. */
    struct VoiceParameters
    {
        // BASE: a 2-operator FM stack (op2 modulates op1, op1 is the audible body).
        double bodyRatio = 1.0;       // op2 frequency ratio relative to op1
        float bodyTuneSemitones = 0.0f;
        float bodyIndex = 0.3f;       // FM modulation depth from op2 -> op1
        float bodyFeedback = 0.0f;    // op1 self-feedback ("BASE" character control)
        float bodyLevel = 1.0f;

        // Pitch envelope: quick sweep applied on top of the body's base pitch.
        float pitchEnvAmountSemitones = 0.0f;
        float pitchEnvDecayMs = 40.0f;

        // NOIS: broadband noise, its own tone filter and envelope.
        float noiseLevel = 0.0f;
        fm::FilterType noiseFilterType = fm::FilterType::highPass;
        float noiseFilterCutoffHz = 2000.0f;
        juce::ADSR::Parameters noiseEnvelope;

        // TRAN: a very short noise burst / click, its own level and decay.
        float transientLevel = 0.0f;
        float transientDecayMs = 5.0f;

        // Shared amp envelope, output filter and drive.
        juce::ADSR::Parameters ampEnvelope;

        fm::FilterType filterType = fm::FilterType::lowPass;
        float filterCutoffHz = 12000.0f;
        float filterResonance = 0.707f;
        float filterEnvAmount = 0.0f;
        float filterKeyTrack = 0.0f;
        juce::ADSR::Parameters filterEnvelope;

        float driveAmount = 0.0f;
        float outputGain = 1.0f;
    };

    class FMDrumVoice : public juce::SynthesiserVoice
    {
    public:
        explicit FMDrumVoice(const VoiceParameters& paramsRef) : params(paramsRef) {}

        using juce::SynthesiserVoice::renderNextBlock;

        bool canPlaySound(juce::SynthesiserSound*) override { return true; }

        void prepare(const juce::dsp::ProcessSpec& spec);

        void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*,
                        int currentPitchWheelPosition) override;
        void stopNote(float velocity, bool allowTailOff) override;
        void pitchWheelMoved(int) override {}
        void controllerMoved(int, int) override {}
        void aftertouchChanged(int) override {}
        void channelPressureChanged(int) override {}

        void renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;

        bool isVoiceActive() const override { return active; }

    private:
        void applyParametersForCurrentNote();

        const VoiceParameters& params;

        fm::FMOperator bodyOp1; // carrier
        fm::FMOperator bodyOp2; // modulator

        juce::ADSR pitchEnvelope;

        juce::Random noiseRng;
        juce::dsp::StateVariableTPTFilter<float> noiseToneFilter;
        juce::ADSR noiseEnvelope;

        juce::ADSR transientEnvelope;

        fm::MultiModeFilter filter;
        juce::ADSR ampEnvelope;
        fm::Drive drive;

        double sampleRate = 44100.0;
        float currentVelocity = 1.0f;
        float baseNote = 60.0f;

        bool active = false;
    };
}
