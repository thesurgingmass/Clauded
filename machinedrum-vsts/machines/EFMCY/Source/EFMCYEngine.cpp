#include "EFMCYEngine.h"
#include <cmath>

void EFMCYEngine::prepare(double sr)
{
    sampleRate = sr;
    fm.prepare(sr);
    bodyEnv.prepare(sr);
    modEnv.prepare(sr);
    hpf.prepare(sr);
}

void EFMCYEngine::reset()
{
    fm.reset();
    hpf.reset();
    active = false;
}

void EFMCYEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 7) return;
    ptchHz = v[0]; dec = v[1]; fb = v[2]; hpfHz = v[3];
    mod = v[4]; mfrq = v[5]; mdec = v[6];
}

void EFMCYEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    baseFreq = ptchHz * std::pow(2.0f, pitchOffsetSemitones / 12.0f);
    fm.setFrequencies(baseFreq, mfrq);

    bodyEnv.setTimes(0.0f, 0.0f, dec);
    bodyEnv.trigger();
    modEnv.setTimes(0.0f, 0.0f, mdec);
    modEnv.trigger();

    hpf.setType(md::FilterType::HighPass);
    hpf.setParams(hpfHz, 0.2f);

    active = true;
}

float EFMCYEngine::renderSample()
{
    float index = modEnv.process() * mod;
    float tone = fm.process(index, fb) * bodyEnv.process();
    float shaped = hpf.process(tone);

    if (! bodyEnv.isActive())
        active = false;

    return shaped * velocity;
}
