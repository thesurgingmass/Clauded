#include "PIXTEngine.h"
#include <cmath>
#include <algorithm>

void PIXTEngine::prepare(double sr)
{
    sampleRate = sr;
    bank.prepare(sr);
}

void PIXTEngine::reset()
{
    bank.reset();
    active = false;
}

void PIXTEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 8) return;
    ptchHz = v[0]; dec = v[1]; hard = v[2]; hamr = v[3];
    tune = v[4]; damp = v[5]; size = v[6]; pos = v[7];
}

void PIXTEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    float sizeScale = 1.6f - size * 1.1f;
    float baseFreq = ptchHz * sizeScale * std::pow(2.0f, (pitchOffsetSemitones + tune) / 12.0f);

    static const std::array<float, numModes> ratios {
        1.0f, 1.594f, 2.136f, 2.296f, 2.653f, 2.918f, 3.156f, 3.501f
    };
    bank.trigger(ratios, baseFreq, dec, damp, pos);

    excitationSamples = std::max(1, (int) ((0.015 - hard * 0.013) * sampleRate));
    samplesSinceTrigger = 0;

    active = true;
}

float PIXTEngine::renderSample()
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
