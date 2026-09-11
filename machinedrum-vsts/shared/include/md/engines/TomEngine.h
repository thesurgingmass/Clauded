#pragma once
#include "../MachineEngine.h"
#include "../dsp/Oscillators.h"
#include "../dsp/Envelope.h"
#include "../dsp/Filter.h"
#include "../dsp/Shaper.h"
#include <cmath>
#include <algorithm>

namespace md
{
    /** Shared engine for TRX-XT and TRX-XC, which have identical SYN pages
        on the real hardware: PTCH, DEC, RAMP, RDEC, DAMP, DIST, DTYP. A
        pitch-enveloped oscillator (RAMP/RDEC sweep) is damped by an internal
        lowpass (DAMP closes the cutoff) then driven through one of three
        distortion characters selected by DTYP (0=soft clip, 1=hard clip,
        2=wavefold) at DIST amount. `roundness` (0=edgier triangle-leaning,
        1=rounder sine-leaning) is the only difference between the two
        machines' default character, since the real distinction between
        XT ("extra tom") and XC isn't publicly documented beyond the shared
        param list. */
    class TomEngine : public MachineEngine
    {
    public:
        explicit TomEngine(float roundnessIn) : roundness(roundnessIn) {}

        void prepare(double sr) override
        {
            sampleRate = sr;
            osc.prepare(sr);
            rampEnv.prepare(sr);
            bodyEnv.prepare(sr);
            damp.prepare(sr);
        }

        void reset() override { osc.reset(); damp.reset(); active = false; }

        void setSynParams(const std::vector<float>& v) override
        {
            if (v.size() < 7) return;
            ptchHz = v[0]; dec = v[1]; ramp = v[2]; rdec = v[3];
            dampAmt = v[4]; dist = v[5]; dtyp = v[6];
        }

        void trigger(float velocity01, float pitchOffsetSemitones) override
        {
            velocity = velocity01;
            baseFreq = ptchHz * std::pow(2.0f, pitchOffsetSemitones / 12.0f);

            rampEnv.setAmountAndTime(ramp, rdec);
            rampEnv.trigger();
            bodyEnv.setTimes(0.0f, 0.0f, dec);
            bodyEnv.trigger();

            damp.setType(FilterType::LowPass);
            active = true;
        }

        float renderSample() override
        {
            float semis = rampEnv.process();
            float freq = baseFreq * std::pow(2.0f, semis / 12.0f);
            osc.setFrequency(freq);

            float amp = bodyEnv.process();
            float raw = osc.sine() * roundness + osc.triangle() * (1.0f - roundness);
            osc.tick();
            raw *= amp;

            float cutoff = std::clamp(8000.0f - dampAmt * 7500.0f, 200.0f, 8000.0f);
            damp.setParams(cutoff, 0.05f);
            float damped = damp.process(raw);

            float out = damped;
            if (dist > 0.0f)
            {
                if (dtyp < 0.5f)      out = softClip(out, dist);
                else if (dtyp < 1.5f) out = std::clamp(out * (1.0f + dist * 6.0f), -1.0f, 1.0f);
                else                  out = fold(out * (1.0f + dist * 4.0f));
            }

            if (! bodyEnv.isActive())
                active = false;

            return out * velocity;
        }

        bool isActive() const override { return active; }

    private:
        AnalogOsc osc;
        PitchDropEnvelope rampEnv;
        AHDEnvelope bodyEnv;
        StateVariableFilter damp;

        double sampleRate = 44100.0;
        float roundness;
        float ptchHz = 150.0f, dec = 0.3f, ramp = -12.0f, rdec = 0.05f;
        float dampAmt = 0.3f, dist = 0.0f, dtyp = 0.0f;

        float baseFreq = 150.0f;
        float velocity = 1.0f;
        bool active = false;
    };
}
