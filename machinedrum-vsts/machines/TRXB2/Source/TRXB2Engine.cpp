#include "TRXB2Engine.h"
#include <cmath>
#include <algorithm>

void TRXB2Engine::prepare(double sr)
{
    sampleRate = sr;
    osc.prepare(sr);
    bumpOsc.prepare(sr);
    ampEnv.prepare(sr);
    bumpEnv.prepare(sr);
    snapEnv.prepare(sr);
}

void TRXB2Engine::reset()
{
    osc.reset();
    bumpOsc.reset();
    active = false;
}

void TRXB2Engine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 8) return;
    ptchHz = v[0]; dec = v[1]; bump = v[2]; benv = v[3];
    snap = v[4]; tone = v[5]; tune = v[6]; clip = v[7];
}

void TRXB2Engine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    baseFreq = ptchHz * std::pow(2.0f, (pitchOffsetSemitones + tune) / 12.0f);
    osc.setFrequency(baseFreq);
    bumpOsc.setFrequency(baseFreq * 0.5f);

    ampEnv.setTimes(0.0f, 0.0f, dec);
    ampEnv.trigger();
    bumpEnv.setTimes(0.0f, 0.0f, benv);
    bumpEnv.trigger();
    snapEnv.setTimes(0.0f, 0.0f, 0.006f);
    snapEnv.trigger();

    active = true;
}

float TRXB2Engine::renderSample()
{
    float amp = ampEnv.process();

    float t = std::clamp(tone, 0.0f, 1.0f);
    float body;
    if (t < 0.5f) body = osc.sine() * (1.0f - t * 2.0f) + osc.triangle() * (t * 2.0f);
    else          body = osc.triangle() * (1.0f - (t - 0.5f) * 2.0f) + osc.saw() * ((t - 0.5f) * 2.0f);
    body *= amp;
    osc.tick();

    float bumpOut = bumpOsc.sine() * bumpEnv.process() * bump;
    bumpOsc.tick();
    float snapOut = noise.next() * snapEnv.process() * snap;

    float out = body + bumpOut + snapOut;
    if (clip > 0.0f) out = md::softClip(out, clip);

    if (! ampEnv.isActive())
        active = false;

    return out * velocity;
}
