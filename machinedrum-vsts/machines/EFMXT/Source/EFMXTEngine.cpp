#include "EFMXTEngine.h"
#include <cmath>

void EFMXTEngine::prepare(double sr)
{
    sampleRate = sr;
    fm.prepare(sr);
    rampEnv.prepare(sr);
    bodyEnv.prepare(sr);
    noiseEnv.prepare(sr);
    clickEnv.prepare(sr);
}

void EFMXTEngine::reset()
{
    fm.reset();
    active = false;
}

void EFMXTEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 8) return;
    ptchHz = v[0]; dec = v[1]; ramp = v[2]; rdec = v[3];
    mod = v[4]; mfrq = v[5]; ndec = v[6]; clic = v[7];
}

void EFMXTEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    baseFreq = ptchHz * std::pow(2.0f, pitchOffsetSemitones / 12.0f);

    rampEnv.setAmountAndTime(ramp, rdec);
    rampEnv.trigger();
    bodyEnv.setTimes(0.0f, 0.0f, dec);
    bodyEnv.trigger();
    noiseEnv.setTimes(0.0f, 0.0f, ndec);
    noiseEnv.trigger();
    clickEnv.setTimes(0.0f, 0.0f, 0.004f);
    clickEnv.trigger();

    active = true;
}

float EFMXTEngine::renderSample()
{
    float semis = rampEnv.process();
    float freq = baseFreq * std::pow(2.0f, semis / 12.0f);
    fm.setFrequencies(freq, mfrq);

    float body = bodyEnv.process();
    float tone = fm.process(mod * body, 0.0f) * body;

    float noiseOut = noise.next() * noiseEnv.process() * 0.4f;
    float click = noise.next() * clickEnv.process() * clic;

    if (! bodyEnv.isActive())
        active = false;

    return (tone + noiseOut + click) * velocity;
}
