#include "EnvelopeEngine.h"
#include <cmath>

namespace vantage
{
    namespace
    {
        constexpr float kStageEndThreshold = 0.001f;
    }

    float EnvelopeEngine::coefficientFor(float timeSeconds, double sampleRate)
    {
        if (timeSeconds <= 0.0005f)
            return 0.0f;

        // Reach 99.9% of the target within `timeSeconds` (a snappy, decisive
        // curve rather than a slow asymptotic creep).
        constexpr float kReach = 6.9077553f; // -ln(1 - 0.999)
        return std::exp(-kReach / (timeSeconds * static_cast<float>(sampleRate)));
    }

    void EnvelopeEngine::prepare(double newSampleRate)
    {
        sampleRate = newSampleRate;
        reset();
    }

    void EnvelopeEngine::reset()
    {
        stage = Stage::Idle;
        value = 0.0f;
    }

    void EnvelopeEngine::setParameters(float attackSeconds, float decaySeconds, float sustain01, float releaseSeconds)
    {
        attackCoeff = coefficientFor(attackSeconds, sampleRate);
        decayCoeff = coefficientFor(decaySeconds, sampleRate);
        releaseCoeff = coefficientFor(releaseSeconds, sampleRate);
        sustainLevel = sustain01;
    }

    void EnvelopeEngine::noteOn()
    {
        stage = Stage::Attack;
    }

    void EnvelopeEngine::noteOff()
    {
        if (stage != Stage::Idle)
            stage = Stage::Release;
    }

    float EnvelopeEngine::renderSample()
    {
        switch (stage)
        {
            case Stage::Attack:
                value += (1.0f - attackCoeff) * (1.0f - value);
                if (value >= 1.0f - kStageEndThreshold)
                {
                    value = 1.0f;
                    stage = Stage::Decay;
                }
                break;

            case Stage::Decay:
                value += (1.0f - decayCoeff) * (sustainLevel - value);
                if (std::abs(value - sustainLevel) <= kStageEndThreshold)
                {
                    value = sustainLevel;
                    stage = Stage::Sustain;
                }
                break;

            case Stage::Sustain:
                value = sustainLevel;
                break;

            case Stage::Release:
                value += (1.0f - releaseCoeff) * (0.0f - value);
                if (value <= kStageEndThreshold)
                {
                    value = 0.0f;
                    stage = Stage::Idle;
                }
                break;

            case Stage::Idle:
            default:
                value = 0.0f;
                break;
        }

        return value;
    }
}
