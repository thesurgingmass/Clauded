#pragma once

#include "RateLevelEnvelope.h"

namespace fs
{
    /**
        A single phase-modulated sine operator — the building block the FM
        engine's 8 operators are made of, in the spirit of DX/FS-series
        Yamaha FM synths.

        Frequency is either a ratio (coarse multiplier + fine offset in
        semitones) against the voice's fundamental, or a fixed Hz value
        (both modes exist on the real hardware, e.g. for bell/metallic
        partials that shouldn't track pitch).

        Modulation input is phase modulation (added directly to the running
        phase, DX-style) rather than frequency modulation, which is what
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
        void setFixedFrequency(bool useFixed, double fixedHz);
        void setLevel(float newLevel) { level = newLevel; }
        void setFeedbackAmount(float newAmount) { feedbackAmount = newAmount; }

        void setEnvelope(const RateLevelEnvelope::Parameters& params) { envelope.setParameters(params); }

        /** True while this operator's envelope is still producing sound. */
        bool isEnvelopeActive() const { return envelope.isActive(); }

        /**
            Renders one sample.

            @param baseFrequencyHz  the voice's current fundamental, before
                                     this operator's ratio is applied (ignored
                                     in fixed-frequency mode)
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
        bool fixedFrequencyMode = false;
        double fixedFrequencyHz = 440.0;

        float level = 1.0f;
        float feedbackAmount = 0.0f;
        float previousOutput = 0.0f;

        RateLevelEnvelope envelope;
    };
}
