#include "TRXMAEngine.h"
#include <cmath>
#include <algorithm>

void TRXMAEngine::prepare(double sr)
{
    sampleRate = sr;
    ampEnv.prepare(sr);
    hardEnv.prepare(sr);
    bandpass.prepare(sr);
    damp.prepare(sr);
}

void TRXMAEngine::reset()
{
    bandpass.reset();
    damp.reset();
    active = false;
}

void TRXMAEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 8) return;
    att = v[0]; sus = v[1]; rev = v[2]; dampAmt = v[3];
    ratl = v[4]; rtyp = v[5]; tone = v[6]; hard = v[7];
}

void TRXMAEngine::trigger(float velocity01, float)
{
    velocity = velocity01;
    ampEnv.setTimes(att, sus, rev);
    ampEnv.trigger();
    hardEnv.setTimes(0.0f, 0.0f, 0.01f);
    hardEnv.trigger();

    bandpass.setType(md::FilterType::BandPass);
    damp.setType(md::FilterType::LowPass);

    active = true;
}

float TRXMAEngine::renderSample()
{
    float raw = noise.next();

    bandpass.setParams(tone, 0.4f);
    float toned = bandpass.process(raw);

    float cutoff = std::max(500.0f, 12000.0f - dampAmt * 10000.0f);
    damp.setParams(cutoff, 0.1f);
    float damped = damp.process(toned);

    tremPhase += md::twoPi * 35.0f / (float) sampleRate;
    if (tremPhase >= md::twoPi) tremPhase -= md::twoPi;
    float smoothRattle = 0.5f + 0.5f * std::sin(tremPhase);

    if (--randHoldCounter <= 0)
    {
        randHoldValue = 0.5f + 0.5f * noise.next();
        randHoldCounter = 1 + (int) (sampleRate * 0.004);
    }

    float rattleShape = smoothRattle * (1.0f - rtyp) + randHoldValue * rtyp;
    float rattleGain = 1.0f - ratl * (1.0f - rattleShape);

    float amp = ampEnv.process();
    float hardOut = hardEnv.process() * hard * noise.next();

    if (! ampEnv.isActive())
        active = false;

    return (damped * rattleGain * amp + hardOut) * velocity;
}
