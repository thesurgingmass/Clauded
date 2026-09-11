#include "PIMAEngine.h"
#include <cmath>
#include <algorithm>

void PIMAEngine::prepare(double sr)
{
    sampleRate = sr;
    ampEnv.prepare(sr);
    bandpass.prepare(sr);
}

void PIMAEngine::reset()
{
    bandpass.reset();
    gateLevel = gateTarget = 0.0f;
    gateCounter = 0;
    active = false;
}

void PIMAEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 5) return;
    grns = v[0]; dec = v[1]; glen = v[2]; size = v[3]; hard = v[4];
}

void PIMAEngine::trigger(float velocity01, float)
{
    velocity = velocity01;
    ampEnv.setTimes(0.001f, 0.0f, dec);
    ampEnv.trigger();

    bandpass.setType(md::FilterType::BandPass);
    gateCounter = 0;

    active = true;
}

float PIMAEngine::renderSample()
{
    if (--gateCounter <= 0)
    {
        gateTarget = (gateNoise.next() > (0.6f - grns * 0.5f)) ? 1.0f : 0.0f;
        float baseLen = (0.02f - glen * 0.015f) * (0.5f + std::fabs(gateNoise.next()) * 0.5f);
        gateCounter = std::max(1, (int) (baseLen * sampleRate));
    }

    float smoothing = 0.05f + hard * 0.9f;
    gateLevel += (gateTarget - gateLevel) * smoothing;

    float freq = 2000.0f + (1.0f - size) * 6000.0f;
    bandpass.setParams(freq, 0.3f);
    float shaped = bandpass.process(noise.next());

    float amp = ampEnv.process();

    if (! ampEnv.isActive())
        active = false;

    return shaped * gateLevel * amp * velocity;
}
