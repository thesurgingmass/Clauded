#pragma once
#include <cmath>
#include <algorithm>

namespace md
{
    enum class FilterType { LowPass, HighPass, BandPass };

    /** Zero-delay-feedback state variable filter -- used for the shared
        FLT page (FREQ/RES) on every machine. */
    class StateVariableFilter
    {
    public:
        void prepare(double sr) { sampleRate = sr; }
        void reset() { low = band = 0.0f; }

        void setType(FilterType t) { type = t; }

        void setParams(float cutoffHz, float resonance01)
        {
            cutoffHz = std::clamp(cutoffHz, 20.0f, (float) sampleRate * 0.49f);
            g = std::tan((float) M_PI * cutoffHz / (float) sampleRate);
            k = 2.0f - 1.98f * std::clamp(resonance01, 0.0f, 1.0f);
        }

        float process(float in)
        {
            float hp = (in - k * band - low) / (1.0f + g * (g + k));
            band = g * hp + band;
            low  = g * band + low;

            switch (type)
            {
                case FilterType::LowPass:  return low;
                case FilterType::HighPass: return hp;
                case FilterType::BandPass: return band;
            }
            return low;
        }

    private:
        double sampleRate = 44100.0;
        float g = 0.1f, k = 1.0f;
        float low = 0.0f, band = 0.0f;
        FilterType type = FilterType::LowPass;
    };
}
