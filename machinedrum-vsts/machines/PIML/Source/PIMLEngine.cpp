#include "PIMLEngine.h"
#include <cmath>
#include <algorithm>

void PIMLEngine::prepare(double sr)
{
    sampleRate = sr;
    bank.prepare(sr);
}

void PIMLEngine::reset()
{
    bank.reset();
    active = false;
}

void PIMLEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 4) return;
    ptchHz = v[0]; dec = v[1]; hard = v[2]; tens = v[3];
}

void PIMLEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    float baseFreq = ptchHz * (0.6f + tens * 1.2f) * std::pow(2.0f, pitchOffsetSemitones / 12.0f);

    static const std::array<float, numModes> ratios { 1.0f, 2.76f, 5.4f, 8.9f, 13.3f };
    bank.trigger(ratios, baseFreq, dec, 0.2f, 0.4f);

    excitationSamples = std::max(1, (int) ((0.01 - hard * 0.008) * sampleRate));
    samplesSinceTrigger = 0;

    active = true;
}

float PIMLEngine::renderSample()
{
    float excitation = 0.0f;
    if (samplesSinceTrigger < excitationSamples)
    {
        float t = (float) samplesSinceTrigger / (float) excitationSamples;
        excitation = noise.next() * (1.0f - t);
    }
    ++samplesSinceTrigger;

    float out = bank.process(excitation);

    if (! bank.isActive() && samplesSinceTrigger >= excitationSamples)
        active = false;

    return out * velocity * 2.5f;
}
