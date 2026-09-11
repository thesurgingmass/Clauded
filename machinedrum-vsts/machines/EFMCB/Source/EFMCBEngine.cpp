#include "EFMCBEngine.h"
#include <cmath>

void EFMCBEngine::prepare(double sr)
{
    sampleRate = sr;
    fm.prepare(sr);
    snapOsc.prepare(sr);
    bodyEnv.prepare(sr);
    modEnv.prepare(sr);
    snapEnv.prepare(sr);
}

void EFMCBEngine::reset()
{
    fm.reset();
    snapOsc.reset();
    active = false;
}

void EFMCBEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 7) return;
    ptchHz = v[0]; dec = v[1]; snap = v[2]; fb = v[3];
    mod = v[4]; mfrq = v[5]; mdec = v[6];
}

void EFMCBEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    baseFreq = ptchHz * std::pow(2.0f, pitchOffsetSemitones / 12.0f);
    fm.setFrequencies(baseFreq, mfrq);
    snapOsc.setFrequency(baseFreq * 1.48f);

    bodyEnv.setTimes(0.0f, 0.0f, dec);
    bodyEnv.trigger();
    modEnv.setTimes(0.0f, 0.0f, mdec);
    modEnv.trigger();
    snapEnv.setTimes(0.0f, 0.0f, 0.015f);
    snapEnv.trigger();

    active = true;
}

float EFMCBEngine::renderSample()
{
    float index = modEnv.process() * mod;
    float tone = fm.process(index, fb) * bodyEnv.process();
    float snapOut = snapOsc.process() * snapEnv.process() * snap;

    if (! bodyEnv.isActive())
        active = false;

    return (tone + snapOut) * velocity;
}
