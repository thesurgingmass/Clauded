#include "TRXRSEngine.h"
#include <cmath>

void TRXRSEngine::prepare(double sr)
{
    sampleRate = sr;
    osc.prepare(sr);
    ampEnv.prepare(sr);
}

void TRXRSEngine::reset()
{
    osc.reset();
    active = false;
}

void TRXRSEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 3) return;
    ptchHz = v[0]; dec = v[1]; dist = v[2];
}

void TRXRSEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    baseFreq = ptchHz * std::pow(2.0f, pitchOffsetSemitones / 12.0f);
    osc.setFrequency(baseFreq);

    ampEnv.setTimes(0.0f, 0.0f, dec);
    ampEnv.trigger();

    active = true;
}

float TRXRSEngine::renderSample()
{
    float amp = ampEnv.process();
    float tone = osc.triangle() * amp;
    osc.tick();
    float click = noise.next() * amp * 0.3f;

    float out = tone + click;
    if (dist > 0.0f) out = md::softClip(out, dist);

    if (! ampEnv.isActive())
        active = false;

    return out * velocity;
}
