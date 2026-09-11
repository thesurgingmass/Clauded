#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace fm
{
    /**
        A single phase-modulated sine operator, the building block both the
        FM TONE and FM DRUM voices are made of.

        Frequency is expressed as a ratio (coarse multiplier + fine offset in
        semitones) against a base frequency the voice supplies each block, so
        the same operator code works whether it's tracking a MIDI note (FM
        TONE) or a fixed/sweeping pitch (FM DRUM's body oscillator).

        Modulation input is phase modulation (added directly to the running
        phase, DX7-style) rather than frequency modulation, which is what
        gives FM synths their characteristic timbre and keeps sidebands
        harmonically related to the carrier.
    */
    class FMOperator
    {
    public:
        void prepare(double newSampleRate);
        void reset();

        void noteOn();
        void noteOff();

        void setRatio(double coarseMultiplier, double fineSemitones);
        void setLevel(float newLevel) { level = newLevel; }
        void setFeedbackAmount(float newAmount) { feedbackAmount = newAmount; }

        void setEnvelope(const juce::ADSR::Parameters& params);

        /** True while this operator's envelope is still producing sound. */
        bool isEnvelopeActive() const { return envelope.isActive(); }

        /**
            Renders one sample.

            @param baseFrequencyHz  the voice's current fundamental, before
                                     this operator's ratio is applied
            @param phaseModulation  summed phase-modulation input from any
                                     operators that modulate this one, in
                                     radians
        */
        float renderSample(double baseFrequencyHz, float phaseModulation);

    private:
        double sampleRate = 44100.0;
        double phase = 0.0;
        double ratioCoarse = 1.0;
        double ratioFineSemitones = 0.0;

        float level = 1.0f;
        float feedbackAmount = 0.0f;
        float previousOutput = 0.0f;

        juce::ADSR envelope;
    };
}
