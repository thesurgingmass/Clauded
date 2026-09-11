#include "EFMBDEngine.h"
#include <cmath>

void EFMBDEngine::prepare(double sr)
{
    sampleRate = sr;
    carrier.prepare(sr);
    modulator.prepare(sr);
    rampEnv.prepare(sr);
    modEnv.prepare(sr);
    bodyEnv.prepare(sr);
}

void EFMBDEngine::reset()
{
    carrier.reset();
    modulator.reset();
    lastModOut = 0.0f;
    active = false;
}

void EFMBDEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 8) return;
    ptchHz = v[0];
    dec    = v[1];
    ramp   = v[2];
    rdec   = v[3];
    mod    = v[4];
    mfrq   = v[5];
    mdec   = v[6];
    mfb    = v[7];
}

void EFMBDEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    baseFreq = ptchHz * std::pow(2.0f, pitchOffsetSemitones / 12.0f);

    rampEnv.setAmountAndTime(ramp, rdec);
    rampEnv.trigger();
    modEnv.setTimes(0.0f, 0.0f, mdec);
    modEnv.trigger();
    bodyEnv.setTimes(0.0f, 0.0f, dec);
    bodyEnv.trigger();

    active = true;
}

float EFMBDEngine::renderSample()
{
    float semis = rampEnv.process();
    float freq = baseFreq * std::pow(2.0f, semis / 12.0f);
    carrier.setFrequency(freq);
    modulator.setFrequency(freq * mfrq);

    float index = modEnv.process() * mod;
    float modOut = modulator.process(lastModOut * mfb);
    lastModOut = modOut;

    float body = bodyEnv.process();
    float out = carrier.process(modOut * index) * body;

    if (! bodyEnv.isActive())
        active = false;

    return out * velocity;
}
