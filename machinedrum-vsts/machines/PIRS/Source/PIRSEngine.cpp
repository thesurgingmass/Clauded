#include "PIRSEngine.h"
#include <cmath>
#include <algorithm>

void PIRSEngine::prepare(double sr)
{
    sampleRate = sr;
    bank.prepare(sr);
    ringFilter.prepare(sr);
    ringEnv.prepare(sr);
}

void PIRSEngine::reset()
{
    bank.reset();
    ringFilter.reset();
    active = false;
}

void PIRSEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 6) return;
    ptchHz = v[0]; dec = v[1]; hard = v[2]; ring = v[3]; rvol = v[4]; rdec = v[5];
}

void PIRSEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    float baseFreq = ptchHz * std::pow(2.0f, pitchOffsetSemitones / 12.0f);

    static const std::array<float, numModes> ratios { 1.0f, 1.8f, 2.4f, 3.2f };
    bank.trigger(ratios, baseFreq, dec, 0.5f, 0.5f);

    ringFilter.setType(md::FilterType::BandPass);
    ringFilter.setParams(baseFreq * 5.0f, 0.9f);
    ringEnv.setTimes(0.0f, 0.0f, rdec);
    ringEnv.trigger();

    excitationSamples = std::max(1, (int) ((0.006 - hard * 0.005) * sampleRate));
    samplesSinceTrigger = 0;

    active = true;
}

float PIRSEngine::renderSample()
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

    return (body * 2.5f + ringOut) * velocity;
}
