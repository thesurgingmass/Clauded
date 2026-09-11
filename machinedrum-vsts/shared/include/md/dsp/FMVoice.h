#pragma once
#include "Oscillators.h"

namespace md
{
    /** Reusable 2-operator FM core (sine carrier phase-modulated by a sine
        modulator, with modulator self-feedback) shared by every EFM-family
        engine. Each machine wraps this with its own envelopes/extra layers. */
    class FM2Op
    {
    public:
        void prepare(double sr) { carrier.prepare(sr); modulator.prepare(sr); }
        void reset() { carrier.reset(); modulator.reset(); lastModOut = 0.0f; }

        void setFrequencies(float carrierHz, float modRatio)
        {
            carrier.setFrequency(carrierHz);
            modulator.setFrequency(carrierHz * modRatio);
        }

        float process(float indexAmt, float feedbackAmt)
        {
            float modOut = modulator.process(lastModOut * feedbackAmt);
            lastModOut = modOut;
            return carrier.process(modOut * indexAmt);
        }

    private:
        FMOperator carrier, modulator;
        float lastModOut = 0.0f;
    };
}
