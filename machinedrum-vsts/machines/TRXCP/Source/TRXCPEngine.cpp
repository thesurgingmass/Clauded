#include "TRXCPEngine.h"
#include <cmath>
#include <algorithm>

void TRXCPEngine::prepare(double sr)
{
    sampleRate = sr;
    tailEnv.prepare(sr);
    toneFilter.prepare(sr);
    burstLenSamples = std::max(1, (int) (0.006 * sr));

    for (int i = 0; i < numCombs; ++i)
    {
        combBuf[(size_t) i].assign((size_t) (sr * 0.05), 0.0f);
        combWrite[(size_t) i] = 0;
        combLast[(size_t) i] = 0.0f;
    }
}

void TRXCPEngine::reset()
{
    toneFilter.reset();
    for (auto& buf : combBuf) std::fill(buf.begin(), buf.end(), 0.0f);
    active = false;
}

void TRXCPEngine::setSynParams(const std::vector<float>& v)
{
    if (v.size() < 8) return;
    clpy = v[0]; tone = v[1]; hard = v[2]; rich = v[3];
    rate = v[4]; room = v[5]; rsiz = v[6]; rtun = v[7];
}

void TRXCPEngine::trigger(float velocity01, float)
{
    velocity = velocity01;
    int spacingSamples = std::max(1, (int) (clpy * 0.001 * sampleRate));
    for (int i = 0; i < (int) burstStart.size(); ++i)
        burstStart[(size_t) i] = i * spacingSamples;
    samplesSinceTrigger = 0;

    tailEnv.setTimes(0.0f, (float) burstStart.back() / (float) sampleRate, 0.3f);
    tailEnv.trigger();

    toneFilter.setType(md::FilterType::BandPass);

    active = true;
}

float TRXCPEngine::renderSample()
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
    float dry = noise.next() * (burstAmp + tailAmp * 0.4f);
    if (rich > 0.0f)
        dry += noise.next() * (burstAmp + tailAmp * 0.4f) * rich * 0.6f;

    toneFilter.setParams(tone, 0.3f);
    dry = toneFilter.process(dry);

    if (hard > 0.0f)
        dry = md::softClip(dry, hard);

    roomPhase += md::twoPi * rate / (float) sampleRate;
    if (roomPhase >= md::twoPi) roomPhase -= md::twoPi;

    float wet = 0.0f;
    for (int i = 0; i < numCombs; ++i)
    {
        auto& buf = combBuf[(size_t) i];
        float baseDelay = (0.006f + 0.012f * rsiz) * (float) (i + 1);
        float modDepth = 0.0008f * std::sin(roomPhase + (float) i);
        int delaySamples = std::clamp((int) ((baseDelay + modDepth) * sampleRate), 1, (int) buf.size() - 1);

        size_t readPos = (combWrite[(size_t) i] + buf.size() - (size_t) delaySamples) % buf.size();
        float delayed = buf[readPos];

        float damped = combLast[(size_t) i] + (delayed - combLast[(size_t) i]) * std::clamp(rtun / 8000.0f, 0.0f, 1.0f);
        combLast[(size_t) i] = damped;

        buf[combWrite[(size_t) i]] = dry + damped * 0.4f;
        combWrite[(size_t) i] = (combWrite[(size_t) i] + 1) % buf.size();

        wet += damped;
    }
    wet /= (float) numCombs;

    float out = dry + wet * room;

    if (! tailEnv.isActive() && burstAmp <= 0.0f)
        active = false;

    return out * velocity;
}
