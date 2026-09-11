#include "PIBDEngine.h"
#include <cmath>
#include <algorithm>

void PIBDEngine::prepare(double sr)
{
    sampleRate = sr;
    bank.prepare(sr);
}

void PIBDEngine::reset()
{
    bank.reset();
    active = false;
}

void PIBDEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 6) return;
    ptchHz = v[0]; dec = v[1]; hard = v[2]; hamr = v[3]; tens = v[4]; damp = v[5];
}

void PIBDEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    float baseFreq = ptchHz * (0.5f + tens * 1.5f) * std::pow(2.0f, pitchOffsetSemitones / 12.0f);

    static const std::array<float, numModes> ratios {
        1.0f, 1.594f, 2.136f, 2.296f, 2.653f, 2.918f, 3.156f, 3.501f
    };
    bank.trigger(ratios, baseFreq, dec, damp, 0.3f);

    excitationSamples = std::max(1, (int) ((0.015 - hard * 0.013) * sampleRate));
    samplesSinceTrigger = 0;

    active = true;
}

float PIBDEngine::renderSample()
{
    float excitation = 0.0f;
    if (samplesSinceTrigger < excitationSamples)
    {
        float t = (float) samplesSinceTrigger / (float) excitationSamples;
        excitation = noise.next() * (1.0f - t) * hamr;
    }
    ++samplesSinceTrigger;

    float out = bank.process(excitation);

    if (! bank.isActive() && samplesSinceTrigger >= excitationSamples)
        active = false;

    return out * velocity * 2.5f;
}
