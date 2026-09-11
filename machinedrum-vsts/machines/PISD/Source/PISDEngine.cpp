#include "PISDEngine.h"
#include <cmath>
#include <algorithm>

void PISDEngine::prepare(double sr)
{
    sampleRate = sr;
    bank.prepare(sr);
    ringFilter.prepare(sr);
    ringEnv.prepare(sr);
}

void PISDEngine::reset()
{
    bank.reset();
    ringFilter.reset();
    active = false;
}

void PISDEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 7) return;
    ptchHz = v[0]; dec = v[1]; hard = v[2]; ring = v[3];
    tens = v[4]; rvol = v[5]; rdec = v[6];
}

void PISDEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    float baseFreq = ptchHz * (0.5f + tens * 1.5f) * std::pow(2.0f, pitchOffsetSemitones / 12.0f);

    static const std::array<float, numModes> ratios { 1.0f, 1.5f, 1.79f, 2.34f, 2.65f, 3.14f };
    bank.trigger(ratios, baseFreq, dec, 0.4f, 0.4f);

    ringFilter.setType(md::FilterType::BandPass);
    ringFilter.setParams(baseFreq * 4.2f, 0.9f);
    ringEnv.setTimes(0.0f, 0.0f, rdec);
    ringEnv.trigger();

    excitationSamples = std::max(1, (int) ((0.008 - hard * 0.006) * sampleRate));
    samplesSinceTrigger = 0;

    active = true;
}

float PISDEngine::renderSample()
{
    float excitation = 0.0f;
    if (samplesSinceTrigger < excitationSamples)
    {
        float t = (float) samplesSinceTrigger / (float) excitationSamples;
        excitation = noise.next() * (1.0f - t);
    }
    ++samplesSinceTrigger;

    float body = bank.process(excitation);
    float ringOut = ringFilter.process(noise.next() * 0.3f) * ringEnv.process() * ring * rvol;

    if (! bank.isActive() && ! ringEnv.isActive() && samplesSinceTrigger >= excitationSamples)
        active = false;

    return (body * 2.0f + ringOut) * velocity;
}
