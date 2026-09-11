#pragma once
#include "../MachineEngine.h"
#include "../dsp/Oscillators.h"
#include "../dsp/Filter.h"
#include "../dsp/Envelope.h"
#include <array>
#include <cmath>
#include <algorithm>

namespace md
{
    /** Shared engine for PI-HH, PI-RC and PI-CC, which all share the same
        8-slot SYN layout: PTCH, DEC, <hardness-or-collision>, RING, AG, AU,
        BR, <damp-or-grab>. A 10-mode inharmonic resonator bank forms the
        metallic body; AG fades in progressively more modes (density), AU
        scales overall output level, and BR tilts weight toward the
        higher/brighter modes. Slot 3 (RING) blends in a separate longer
        sustain layer, and slot 8 (CLOS on the hi-hat / GRAB on the
        cymbals) shortens the decay for a damped/choked character. The
        exact hardware semantics of AG/AU/BR beyond their documented names
        ("amplitude modulation"-adjacent names aren't given for this family)
        are not publicly detailed, so this mapping is a physically-plausible
        approximation. */
    class CymbalEngine : public MachineEngine
    {
    public:
        void prepare(double sr) override
        {
            sampleRate = sr;
            for (auto& f : filters) f.prepare(sr);
            for (auto& e : envs) e.prepare(sr);
            ringFilter.prepare(sr);
            ringEnv.prepare(sr);
        }

        void reset() override
        {
            for (auto& f : filters) f.reset();
            ringFilter.reset();
            active = false;
        }

        void setSynParams(const std::vector<float>& v) override
        {
            if (v.size() < 8) return;
            ptchHz = v[0]; dec = v[1]; hardness = v[2]; ring = v[3];
            ag = v[4]; au = v[5]; br = v[6]; damp = v[7];
        }

        void trigger(float velocity01, float pitchOffsetSemitones) override
        {
            velocity = velocity01;
            float baseFreq = ptchHz * std::pow(2.0f, pitchOffsetSemitones / 12.0f);
            static constexpr std::array<float, numModes> ratios {
                1.0f, 1.342f, 1.2312f, 1.6532f, 1.9758f, 2.4765f, 2.9027f, 3.3421f, 3.8129f, 4.2058f
            };

            float effectiveDec = dec / (1.0f + damp * 4.0f);
            for (int i = 0; i < numModes; ++i)
            {
                filters[(size_t) i].setType(FilterType::BandPass);
                filters[(size_t) i].setParams(std::clamp(baseFreq * ratios[(size_t) i], 20.0f, 20000.0f), 0.92f);

                float modeDecay = std::max(0.01f, effectiveDec / (1.0f + (float) i * 0.3f));
                envs[(size_t) i].setTimes(0.0f, 0.0f, modeDecay);
                envs[(size_t) i].trigger();

                float densityWeight = std::clamp(ag * (float) numModes - (float) i, 0.0f, 1.0f);
                float brightnessWeight = 1.0f + br * (float) i * 0.15f;
                weight[(size_t) i] = densityWeight * brightnessWeight;
            }

            ringFilter.setType(FilterType::BandPass);
            ringFilter.setParams(baseFreq * 3.5f, 0.85f);
            ringEnv.setTimes(0.0f, 0.0f, std::max(0.05f, effectiveDec * 1.5f));
            ringEnv.trigger();

            excitationSamples = std::max(1, (int) ((0.012f - hardness * 0.01f) * (float) sampleRate));
            samplesSinceTrigger = 0;

            active = true;
        }

        float renderSample() override
        {
            float excitation = 0.0f;
            if (samplesSinceTrigger < excitationSamples)
            {
                float t = (float) samplesSinceTrigger / (float) excitationSamples;
                excitation = noise.next() * (1.0f - t);
            }
            ++samplesSinceTrigger;

            float sum = 0.0f;
            for (int i = 0; i < numModes; ++i)
                sum += filters[(size_t) i].process(excitation) * envs[(size_t) i].process() * weight[(size_t) i];
            sum /= (float) numModes;

            float ringOut = ringFilter.process(noise.next() * 0.2f) * ringEnv.process() * ring;

            bool anyActive = ringEnv.isActive();
            for (auto& e : envs) anyActive = anyActive || e.isActive();
            if (! anyActive && samplesSinceTrigger >= excitationSamples)
                active = false;

            return (sum * 3.0f + ringOut) * au * velocity;
        }

        bool isActive() const override { return active; }

    private:
        static constexpr int numModes = 10;
        std::array<StateVariableFilter, numModes> filters;
        std::array<AHDEnvelope, numModes> envs;
        std::array<float, numModes> weight {};
        NoiseGen noise;
        StateVariableFilter ringFilter;
        AHDEnvelope ringEnv;

        double sampleRate = 44100.0;
        float ptchHz = 2000.0f, dec = 0.8f, hardness = 0.4f, ring = 0.3f;
        float ag = 0.6f, au = 0.7f, br = 0.3f, damp = 0.2f;

        int excitationSamples = 0, samplesSinceTrigger = 0;
        float velocity = 1.0f;
        bool active = false;
    };
}
