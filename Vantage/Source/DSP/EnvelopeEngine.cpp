#include "EnvelopeEngine.h"

namespace vantage
{
    void EnvelopeEngine::prepare(double newSampleRate)
    {
        adsr.setSampleRate(newSampleRate);
        reset();
    }

    void EnvelopeEngine::reset()
    {
        adsr.reset();
    }

    void EnvelopeEngine::setParameters(float attackSeconds, float decaySeconds, float sustain01, float releaseSeconds)
    {
        adsr.setParameters({ attackSeconds, decaySeconds, sustain01, releaseSeconds });
    }

    void EnvelopeEngine::noteOn()
    {
        adsr.noteOn();
    }

    void EnvelopeEngine::noteOff()
    {
        adsr.noteOff();
    }

    float EnvelopeEngine::renderSample()
    {
        return adsr.getNextSample();
    }
}
