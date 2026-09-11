#include "TRXCLEngine.h"
#include <cmath>

void TRXCLEngine::prepare(double sr)
{
    sampleRate = sr;
    osc1.prepare(sr);
    osc2.prepare(sr);
    ampEnv.prepare(sr);
    clickEnv.prepare(sr);
}

void TRXCLEngine::reset()
{
    osc1.reset(); osc2.reset();
    active = false;
}

void TRXCLEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 6) return;
    ptchHz = v[0]; dec = v[1]; dual = v[2]; enh = v[3]; tune = v[4]; clic = v[5];
}

void TRXCLEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    baseFreq = ptchHz * std::pow(2.0f, pitchOffsetSemitones / 12.0f);
    osc1.setFrequency(baseFreq);
    osc2.setFrequency(baseFreq * std::pow(2.0f, tune / 12.0f));

    ampEnv.setTimes(0.0f, 0.0f, dec);
    ampEnv.trigger();
    clickEnv.setTimes(0.0f, 0.0f, 0.003f);
    clickEnv.trigger();

    active = true;
}

float TRXCLEngine::renderSample()
{
    float amp = ampEnv.process();
    float tone = osc1.sine() * (1.0f - dual * 0.5f) + osc2.sine() * (dual * 0.5f);
    osc1.tick(); osc2.tick();

    if (enh > 0.0f) tone = md::fold(tone * (1.0f + enh * 3.0f));
    tone *= amp;

    float click = noise.next() * clickEnv.process() * clic;

    if (! ampEnv.isActive())
        active = false;

    return (tone + click) * velocity;
}
