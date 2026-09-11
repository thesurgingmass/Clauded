#include "EFMHHEngine.h"
#include <cmath>

void EFMHHEngine::prepare(double sr)
{
    sampleRate = sr;
    fm.prepare(sr);
    bodyEnv.prepare(sr);
    modEnv.prepare(sr);
}

void EFMHHEngine::reset()
{
    fm.reset();
    tremPhase = 0.0f;
    active = false;
}

void EFMHHEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 8) return;
    ptchHz = v[0]; dec = v[1]; trem = v[2]; tfrq = v[3];
    mod = v[4]; mfrq = v[5]; mdec = v[6]; fb = v[7];
}

void EFMHHEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    baseFreq = ptchHz * std::pow(2.0f, pitchOffsetSemitones / 12.0f);
    fm.setFrequencies(baseFreq, mfrq);

    bodyEnv.setTimes(0.0f, 0.0f, dec);
    bodyEnv.trigger();
    modEnv.setTimes(0.0f, 0.0f, mdec);
    modEnv.trigger();

    active = true;
}

float EFMHHEngine::renderSample()
{
    float index = modEnv.process() * mod;
    float tone = fm.process(index, fb) * bodyEnv.process();

    tremPhase += md::twoPi * tfrq / (float) sampleRate;
    if (tremPhase >= md::twoPi) tremPhase -= md::twoPi;
    float tremGain = 1.0f - trem * 0.5f * (1.0f - std::sin(tremPhase));

    if (! bodyEnv.isActive())
        active = false;

    return tone * tremGain * velocity;
}
