#include "TRXSDEngine.h"
#include <cmath>

void TRXSDEngine::prepare(double sr)
{
    sampleRate = sr;
    osc1.prepare(sr);
    osc2.prepare(sr);
    rampEnv.prepare(sr);
    bodyEnv.prepare(sr);
    noiseEnv.prepare(sr);
}

void TRXSDEngine::reset()
{
    osc1.reset();
    osc2.reset();
    active = false;
}

void TRXSDEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 8) return;
    ptchHz = v[0]; dec = v[1]; ramp = v[2]; rdec = v[3];
    strt = v[4]; nois = v[5]; harm = v[6]; clip = v[7];
}

void TRXSDEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    baseFreq = ptchHz * std::pow(2.0f, pitchOffsetSemitones / 12.0f);
    osc2.setFrequency(baseFreq * 1.5f);

    rampEnv.setAmountAndTime(ramp, rdec);
    rampEnv.trigger();
    bodyEnv.setTimes(0.0f, 0.0f, dec);
    bodyEnv.trigger();

    strtSamples = (int) (strt * 0.001f * sampleRate);
    samplesSinceTrigger = 0;
    noiseStarted = false;

    active = true;
}

float TRXSDEngine::renderSample()
{
    float semis = rampEnv.process();
    float freq = baseFreq * std::pow(2.0f, semis / 12.0f);
    osc1.setFrequency(freq);

    float body = bodyEnv.process();
    float tone = (osc1.triangle() * (1.0f - harm) + osc2.triangle() * harm) * body;
    osc1.tick();
    osc2.tick();

    if (! noiseStarted && samplesSinceTrigger >= strtSamples)
    {
        noiseEnv.setTimes(0.0f, 0.0f, dec * 1.3f);
        noiseEnv.trigger();
        noiseStarted = true;
    }
    ++samplesSinceTrigger;

    float noiseOut = noise.next() * noiseEnv.process() * nois;

    float out = tone * 0.7f + noiseOut;
    if (clip > 0.0f) out = md::softClip(out, clip);

    if (! bodyEnv.isActive() && (! noiseStarted || ! noiseEnv.isActive()))
        active = false;

    return out * velocity;
}
