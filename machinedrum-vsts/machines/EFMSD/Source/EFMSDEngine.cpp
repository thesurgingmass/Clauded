#include "EFMSDEngine.h"
#include <cmath>

void EFMSDEngine::prepare(double sr)
{
    sampleRate = sr;
    fm.prepare(sr);
    bodyEnv.prepare(sr);
    noiseEnv.prepare(sr);
    clickEnv.prepare(sr);
    modEnv.prepare(sr);
}

void EFMSDEngine::reset()
{
    fm.reset();
    active = false;
}

void EFMSDEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 8) return;
    ptchHz = v[0]; dec = v[1]; nois = v[2]; ndec = v[3];
    mod = v[4]; mfrq = v[5]; mdec = v[6]; clic = v[7];
}

void EFMSDEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    baseFreq = ptchHz * std::pow(2.0f, pitchOffsetSemitones / 12.0f);
    fm.setFrequencies(baseFreq, mfrq);

    bodyEnv.setTimes(0.0f, 0.0f, dec);
    bodyEnv.trigger();
    modEnv.setTimes(0.0f, 0.0f, mdec);
    modEnv.trigger();
    noiseEnv.setTimes(0.0f, 0.0f, ndec);
    noiseEnv.trigger();
    clickEnv.setTimes(0.0f, 0.0f, 0.004f);
    clickEnv.trigger();

    active = true;
}

float EFMSDEngine::renderSample()
{
    float index = modEnv.process() * mod;
    float tone = fm.process(index, 0.0f) * bodyEnv.process();

    float noiseOut = noise.next() * noiseEnv.process() * nois;
    float click = noise.next() * clickEnv.process() * clic;

    if (! bodyEnv.isActive() && ! noiseEnv.isActive())
        active = false;

    return (tone * 0.6f + noiseOut * 0.8f + click) * velocity;
}
