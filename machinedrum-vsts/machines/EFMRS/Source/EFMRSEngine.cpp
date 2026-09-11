#include "EFMRSEngine.h"
#include <cmath>

void EFMRSEngine::prepare(double sr)
{
    sampleRate = sr;
    fm.prepare(sr);
    bodyEnv.prepare(sr);
    noiseEnv.prepare(sr);
    modEnv.prepare(sr);
    hpf.prepare(sr);
}

void EFMRSEngine::reset()
{
    fm.reset();
    hpf.reset();
    active = false;
}

void EFMRSEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 8) return;
    ptchHz = v[0]; dec = v[1]; noiseAmt = v[2]; ndec = v[3];
    mod = v[4]; mfrq = v[5]; mdec = v[6]; hpfHz = v[7];
}

void EFMRSEngine::trigger(float velocity01, float pitchOffsetSemitones)
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

    hpf.setType(md::FilterType::HighPass);
    hpf.setParams(hpfHz, 0.15f);

    active = true;
}

float EFMRSEngine::renderSample()
{
    float index = modEnv.process() * mod;
    float tone = fm.process(index, 0.0f) * bodyEnv.process();

    float noiseRaw = noise.next() * noiseEnv.process() * noiseAmt;
    float noiseOut = hpf.process(noiseRaw);

    if (! bodyEnv.isActive() && ! noiseEnv.isActive())
        active = false;

    return (tone * 0.7f + noiseOut) * velocity;
}
