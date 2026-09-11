#include "FMOperator.h"
#include "DspUtils.h"

namespace fm
{
    void FMOperator::prepare(double newSampleRate)
    {
        sampleRate = newSampleRate;
        envelope.setSampleRate(newSampleRate);
        reset();
    }

    void FMOperator::reset()
    {
        phase = 0.0;
        previousOutput = 0.0f;
        envelope.reset();
    }

    void FMOperator::noteOn()
    {
        // Free-running phase, not reset to 0, so re-triggering a held note
        // doesn't click and different operators keep their relative phase
        // relationships (important for the feedback operator especially).
        envelope.noteOn();
    }

    void FMOperator::noteOff()
    {
        envelope.noteOff();
    }

    void FMOperator::setRatio(double coarseMultiplier, double fineSemitones)
    {
        ratioCoarse = coarseMultiplier;
        ratioFineSemitones = fineSemitones;
    }

    void FMOperator::setEnvelope(const juce::ADSR::Parameters& params)
    {
        envelope.setParameters(params);
    }

    float FMOperator::renderSample(double baseFrequencyHz, float phaseModulation)
    {
        const double frequency = baseFrequencyHz * ratioCoarse * semitonesToRatio(ratioFineSemitones);
        const double phaseIncrement = frequency / sampleRate;

        const float feedback = previousOutput * feedbackAmount * 2.0f;
        const float instantaneousPhase =
            static_cast<float>(phase * twoPi) + phaseModulation + feedback;

        const float envelopeValue = envelope.getNextSample();
        const float output = std::sin(instantaneousPhase) * level * envelopeValue;

        previousOutput = output;

        phase += phaseIncrement;
        if (phase >= 1.0)
            phase -= 1.0;

        return output;
    }
}
