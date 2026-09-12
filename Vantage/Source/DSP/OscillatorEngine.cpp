#include "OscillatorEngine.h"
#include "PolyBlep.h"
#include "Wavetable.h"
#include <cmath>

namespace vantage
{
    namespace
    {
        constexpr float kTwoPi = 6.28318530717958647692f;

        constexpr float kMoogDriftDepthCents = 4.0f;
        constexpr float kOberheimDetuneCents = 7.0f;
        constexpr float kRolandChorusRateHz = 0.6f;
        constexpr float kRolandChorusDepthCents = 5.0f;

        float centsToRatio(float cents)
        {
            return std::pow(2.0f, cents / 1200.0f);
        }
    }

    void OscillatorEngine::prepare(double newSampleRate)
    {
        sampleRate = newSampleRate;
        reset();
    }

    void OscillatorEngine::reset()
    {
        phase = 0.0f;
        secondPhase = 0.0f;
        chorusPhase = 0.0f;
        driftValue = 0.0f;
        driftTarget = 0.0f;
        driftCounter = 0;
    }

    void OscillatorEngine::setFrequency(float newFrequencyHz)
    {
        frequencyHz = newFrequencyHz;
        phaseIncrement = static_cast<float>(frequencyHz / sampleRate);
    }

    void OscillatorEngine::advancePhase(float& phaseToAdvance, float increment) const
    {
        phaseToAdvance += increment;
        if (phaseToAdvance >= 1.0f)
            phaseToAdvance -= 1.0f;
        else if (phaseToAdvance < 0.0f)
            phaseToAdvance += 1.0f;
    }

    float OscillatorEngine::renderMoog()
    {
        if (--driftCounter <= 0)
        {
            driftCounter = static_cast<int>(sampleRate * 0.15);
            driftTarget = driftRandom.nextFloat() * 2.0f - 1.0f;
        }
        driftValue += 0.0005f * (driftTarget - driftValue);

        const float increment = phaseIncrement * centsToRatio(driftValue * kMoogDriftDepthCents);
        const float sample = polyBlepSaw(phase, increment);
        advancePhase(phase, increment);
        return sample;
    }

    float OscillatorEngine::renderOberheim()
    {
        const float increment2 = phaseIncrement * centsToRatio(kOberheimDetuneCents);

        const float saw1 = polyBlepSaw(phase, phaseIncrement);
        const float saw2 = polyBlepSaw(secondPhase, increment2);

        advancePhase(phase, phaseIncrement);
        advancePhase(secondPhase, increment2);

        return (saw1 + saw2) * 0.5f;
    }

    float OscillatorEngine::renderRoland()
    {
        advancePhase(chorusPhase, static_cast<float>(kRolandChorusRateHz / sampleRate));
        const float lfo = std::sin(kTwoPi * chorusPhase);

        const float increment = phaseIncrement * centsToRatio(lfo * kRolandChorusDepthCents);
        const float sample = polyBlepSaw(phase, increment);
        advancePhase(phase, increment);
        return sample;
    }

    float OscillatorEngine::renderNord()
    {
        const float saw = polyBlepSaw(phase, phaseIncrement);
        const float square = polyBlepSquare(phase, phaseIncrement, 0.5f);
        advancePhase(phase, phaseIncrement);
        return saw * 0.7f + square * 0.3f;
    }

    float OscillatorEngine::renderWavetable()
    {
        const float sample = getSharedWavetable().sample(phase, wavetablePosition);
        advancePhase(phase, phaseIncrement);
        return sample;
    }

    float OscillatorEngine::renderSample()
    {
        switch (model)
        {
            case OscillatorModel::Moog:       return renderMoog();
            case OscillatorModel::Oberheim:   return renderOberheim();
            case OscillatorModel::Roland:     return renderRoland();
            case OscillatorModel::Nord:       return renderNord();
            case OscillatorModel::Wavetable:  return renderWavetable();
            case OscillatorModel::count:
            default:                          return renderNord();
        }
    }
}
