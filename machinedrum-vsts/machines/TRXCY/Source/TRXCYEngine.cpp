#include "TRXCYEngine.h"
#include <cmath>
#include <algorithm>

void TRXCYEngine::prepare(double sr)
{
    sampleRate = sr;
    for (auto& o : oscs) o.prepare(sr);
    ampEnv.prepare(sr);
    topFilter.prepare(sr);
}

void TRXCYEngine::reset()
{
    for (auto& o : oscs) o.reset();
    topFilter.reset();
    active = false;
}

void TRXCYEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 6) return;
    rich = v[0]; dec = v[1]; top = v[2]; ttun = v[3]; size = v[4]; peak = v[5];
}

void TRXCYEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    float baseFreq = (800.0f - size * 650.0f) * std::pow(2.0f, pitchOffsetSemitones / 12.0f);
    static constexpr std::array<float, 6> ratios { 1.0f, 1.342f, 1.2312f, 1.6532f, 1.9758f, 2.4765f };
    for (size_t i = 0; i < oscs.size(); ++i)
        oscs[i].setFrequency(baseFreq * ratios[i]);

    ampEnv.setTimes(0.0f, 0.0f, dec);
    ampEnv.trigger();

    topFilter.setType(md::FilterType::BandPass);

    active = true;
}

float TRXCYEngine::renderSample()
{
    float raw = 0.0f;
    for (size_t i = 0; i < oscs.size(); ++i)
    {
        float weight = std::clamp(rich * (float) oscs.size() - (float) i, 0.0f, 1.0f);
        raw += oscs[i].square() * weight;
        oscs[i].tick();
    }
    raw /= (float) oscs.size();

    topFilter.setParams(ttun, peak);
    float topOut = topFilter.process(raw);

    float shaped = raw * (1.0f - top) + topOut * top;
    float amp = ampEnv.process();

    if (! ampEnv.isActive())
        active = false;

    return shaped * amp * velocity;
}
