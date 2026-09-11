#include "EFMCPEngine.h"
#include <cmath>
#include <algorithm>

void EFMCPEngine::prepare(double sr)
{
    sampleRate = sr;
    fm.prepare(sr);
    bodyEnv.prepare(sr);
    modEnv.prepare(sr);
    tailEnv.prepare(sr);
    hpf.prepare(sr);
    burstLenSamples = std::max(1, (int) (0.006 * sr));
}

void EFMCPEngine::reset()
{
    fm.reset();
    hpf.reset();
    active = false;
}

void EFMCPEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 8) return;
    ptchHz = v[0]; dec = v[1]; clps = v[2]; cdec = v[3];
    mod = v[4]; mfrq = v[5]; mdec = v[6]; hpfHz = v[7];
}

void EFMCPEngine::trigger(float velocity01, float pitchOffsetSemitones)
{
    velocity = velocity01;
    float freq = ptchHz * std::pow(2.0f, pitchOffsetSemitones / 12.0f);
    fm.setFrequencies(freq, mfrq);

    int spacingSamples = std::max(1, (int) (clps * 0.001 * sampleRate));
    for (int i = 0; i < (int) burstStart.size(); ++i)
        burstStart[(size_t) i] = i * spacingSamples;
    samplesSinceTrigger = 0;

    bodyEnv.setTimes(0.0f, 0.0f, dec);
    bodyEnv.trigger();
    modEnv.setTimes(0.0f, 0.0f, mdec);
    modEnv.trigger();
    tailEnv.setTimes(0.0f, (float) burstStart.back() / (float) sampleRate, cdec);
    tailEnv.trigger();

    hpf.setType(md::FilterType::HighPass);
    hpf.setParams(hpfHz, 0.2f);

    active = true;
}

float EFMCPEngine::renderSample()
{
    float burstAmp = 0.0f;
    for (int bt : burstStart)
    {
        int elapsed = samplesSinceTrigger - bt;
        if (elapsed >= 0 && elapsed < burstLenSamples)
        {
            float t = (float) elapsed / (float) burstLenSamples;
            burstAmp = std::max(burstAmp, (1.0f - t) * (1.0f - t));
        }
    }
    ++samplesSinceTrigger;

    float tailAmp = tailEnv.process();
    float noiseRaw = noise.next() * (burstAmp * 0.9f + tailAmp * 0.5f);
    float noiseOut = hpf.process(noiseRaw);

    float index = modEnv.process() * mod;
    float tone = fm.process(index, 0.0f) * bodyEnv.process();

    if (! tailEnv.isActive() && ! bodyEnv.isActive() && burstAmp <= 0.0f)
        active = false;

    return (noiseOut * 0.9f + tone * 0.3f) * velocity;
}
