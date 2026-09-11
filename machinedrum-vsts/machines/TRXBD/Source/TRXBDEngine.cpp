#include "TRXBDEngine.h"
#include <cmath>

void TRXBDEngine::prepare(double sr)
{
    sampleRate = sr;
    osc.prepare(sr);
    rampEnv.prepare(sr);
    ampEnv.prepare(sr);
    tickEnv.prepare(sr);
}

void TRXBDEngine::reset()
{
    osc.reset();
    active = false;
}

void TRXBDEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 9) return;
    ptchHz = v[0]; dec = v[1]; ramp = v[2]; rdec = v[3];
    hold = v[4]; tick = v[5]; nois = v[6]; dirt = v[7]; dist = v[8];
}

void TRXBDEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    baseFreq = ptchHz * std::pow(2.0f, pitchOffsetSemitones / 12.0f);

    rampEnv.setAmountAndTime(ramp, rdec);
    rampEnv.trigger();
    ampEnv.setTimes(0.0f, hold, dec);
    ampEnv.trigger();
    tickEnv.setTimes(0.0f, 0.0f, 0.003f);
    tickEnv.trigger();

    active = true;
}

float TRXBDEngine::renderSample()
{
    float semis = rampEnv.process();
    float freq = baseFreq * std::pow(2.0f, semis / 12.0f);
    osc.setFrequency(freq);

    float amp = ampEnv.process();
    float body = osc.sine() * amp;
    osc.tick();
    float tickOut = noise.next() * tickEnv.process() * tick;
    float noiseOut = noise.next() * amp * nois;

    float out = body + tickOut + noiseOut;
    if (dirt > 0.0f) out = md::softClip(out, dirt * 0.4f);
    if (dist > 0.0f) out = md::softClip(out, dist);

    if (! ampEnv.isActive())
        active = false;

    return out * velocity;
}
