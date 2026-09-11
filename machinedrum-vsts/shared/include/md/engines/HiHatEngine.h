#pragma once
#include "../MachineEngine.h"
#include "../dsp/Oscillators.h"
#include "../dsp/Envelope.h"
#include "../dsp/Filter.h"
#include <array>
#include <cmath>

namespace md
{
    /** Shared engine for TRX-CH and TRX-OH (closed/open hi-hat), which have
        identical SYN pages: GAP, DEC, HPF, LPF, MTAL. A cluster of square
        oscillators at classic inharmonic ratios (metallic character) is
        blended with white noise (MTAL: 0=noise-only, 1=fully metallic),
        with GAP spreading/detuning the oscillator ratios apart, then shaped
        by independent highpass (HPF) and lowpass (LPF) corners forming the
        passband. The two machines differ only in default DEC (closed =
        short, open = long) -- matching how both share one filter/decay
        structure on the real hardware, gated only by trigger choke groups
        that don't apply to a standalone single-voice instrument. */
    class HiHatEngine : public MachineEngine
    {
    public:
        void prepare(double sr) override
        {
            sampleRate = sr;
            for (auto& o : oscs) o.prepare(sr);
            ampEnv.prepare(sr);
            hp.prepare(sr);
            lp.prepare(sr);
        }

        void reset() override
        {
            for (auto& o : oscs) o.reset();
            hp.reset();
            lp.reset();
            active = false;
        }

        void setSynParams(const std::vector<float>& v) override
        {
            if (v.size() < 5) return;
            gap = v[0]; dec = v[1]; hpfHz = v[2]; lpfHz = v[3]; mtal = v[4];
        }

        void trigger(float velocity01, float pitchOffsetSemitones) override
        {
            velocity = velocity01;
            float baseFreq = 260.0f * std::pow(2.0f, pitchOffsetSemitones / 12.0f);
            static constexpr std::array<float, 6> ratios { 1.0f, 1.342f, 1.2312f, 1.6532f, 1.9758f, 2.4765f };
            for (size_t i = 0; i < oscs.size(); ++i)
            {
                float spread = 1.0f + gap * 0.01f * (float) i;
                oscs[i].setFrequency(baseFreq * ratios[i] * spread);
            }

            ampEnv.setTimes(0.0f, 0.0f, dec);
            ampEnv.trigger();

            hp.setType(FilterType::HighPass);
            lp.setType(FilterType::LowPass);

            active = true;
        }

        float renderSample() override
        {
            float metallic = 0.0f;
            for (auto& o : oscs)
            {
                metallic += o.square() * (1.0f / (float) oscs.size());
                o.tick();
            }

            float noiseOut = noise.next();
            float raw = metallic * mtal + noiseOut * (1.0f - mtal);

            hp.setParams(hpfHz, 0.1f);
            lp.setParams(lpfHz, 0.1f);
            float shaped = lp.process(hp.process(raw));

            float amp = ampEnv.process();

            if (! ampEnv.isActive())
                active = false;

            return shaped * amp * velocity;
        }

        bool isActive() const override { return active; }

    private:
        std::array<AnalogOsc, 6> oscs;
        NoiseGen noise;
        AHDEnvelope ampEnv;
        StateVariableFilter hp, lp;

        double sampleRate = 44100.0;
        float gap = 0.3f, dec = 0.1f, hpfHz = 3000.0f, lpfHz = 12000.0f, mtal = 0.6f;
        float velocity = 1.0f;
        bool active = false;
    };
}
