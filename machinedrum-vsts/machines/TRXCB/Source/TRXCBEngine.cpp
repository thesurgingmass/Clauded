#include "TRXCBEngine.h"
#include <cmath>
#include <algorithm>

void TRXCBEngine::prepare(double sr)
{
    sampleRate = sr;
    osc1.prepare(sr);
    osc2.prepare(sr);
    bumpOsc.prepare(sr);
    ampEnv.prepare(sr);
    bumpEnv.prepare(sr);
    lp.prepare(sr);
}

void TRXCBEngine::reset()
{
    osc1.reset(); osc2.reset(); bumpOsc.reset();
    lp.reset();
    active = false;
}

void TRXCBEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 6) return;
    ptchHz = v[0]; dec = v[1]; enh = v[2]; damp = v[3]; tone = v[4]; bump = v[5];
}

void TRXCBEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    baseFreq = ptchHz * std::pow(2.0f, pitchOffsetSemitones / 12.0f);
    osc1.setFrequency(baseFreq);
    osc2.setFrequency(baseFreq * 1.48f);
    bumpOsc.setFrequency(baseFreq * 0.5f);

    ampEnv.setTimes(0.0f, 0.0f, dec);
    ampEnv.trigger();
    bumpEnv.setTimes(0.0f, 0.0f, 0.05f);
    bumpEnv.trigger();

    lp.setType(md::FilterType::LowPass);

    active = true;
}

float TRXCBEngine::renderSample()
{
    float amp = ampEnv.process();
    float raw = (osc1.square() + osc2.square()) * 0.5f;
    osc1.tick(); osc2.tick();

    if (enh > 0.0f) raw = md::fold(raw * (1.0f + enh * 3.0f));

    float cutoff = std::max(300.0f, tone - damp * (tone - 300.0f));
    lp.setParams(cutoff, 0.15f);
    float shaped = lp.process(raw) * amp;

    float bumpOut = bumpOsc.sine() * bumpEnv.process() * bump;
    bumpOsc.tick();

    if (! ampEnv.isActive())
        active = false;

    return (shaped + bumpOut) * velocity;
}
