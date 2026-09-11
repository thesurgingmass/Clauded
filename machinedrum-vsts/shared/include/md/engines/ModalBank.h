#pragma once
#include "../dsp/Filter.h"
#include "../dsp/Envelope.h"
#include <array>
#include <cmath>
#include <algorithm>

namespace md
{
    /** A small bank of resonant bandpass "modes" excited by a short impulse,
        the shared building block behind every PI-family (physically
        informed) machine: strike a resonant body, hear its natural modes
        ring out and decay. Each mode has its own decay (faster for higher
        modes, scaled by dampAmount) and a strike-position-style amplitude
        weighting (posAmount) -- the classic technique of weighting mode
        amplitudes by where an idealized strike lands on the body. */
    template <int N>
    class ModalBank
    {
    public:
        void prepare(double sr) { for (auto& f : filters) f.prepare(sr); for (auto& e : envs) e.prepare(sr); }
        void reset() { for (auto& f : filters) f.reset(); }

        void trigger(const std::array<float, N>& ratios, float baseFreq,
                     float decaySeconds, float dampAmount, float posAmount)
        {
            for (int i = 0; i < N; ++i)
            {
                float freq = std::clamp(baseFreq * ratios[(size_t) i], 20.0f, 20000.0f);
                filters[(size_t) i].setType(FilterType::BandPass);
                filters[(size_t) i].setParams(freq, 0.93f);

                float modeDecay = std::max(0.005f, decaySeconds / (1.0f + dampAmount * (float) i * 1.5f));
                envs[(size_t) i].setTimes(0.0f, 0.0f, modeDecay);
                envs[(size_t) i].trigger();

                weight[(size_t) i] = 0.4f + 0.6f * std::fabs(std::sin((float) M_PI * (float) (i + 1) * (0.15f + posAmount * 0.35f)));
            }
        }

        float process(float excitation)
        {
            float sum = 0.0f;
            for (int i = 0; i < N; ++i)
                sum += filters[(size_t) i].process(excitation) * envs[(size_t) i].process() * weight[(size_t) i];
            return sum / (float) N;
        }

        bool isActive() const
        {
            for (auto& e : envs) if (e.isActive()) return true;
            return false;
        }

    private:
        std::array<StateVariableFilter, N> filters;
        std::array<AHDEnvelope, N> envs;
        std::array<float, N> weight {};
    };
}
