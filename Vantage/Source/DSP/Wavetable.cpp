#include "Wavetable.h"
#include <algorithm>
#include <cmath>

namespace vantage
{
    namespace
    {
        constexpr float kTwoPi = 6.28318530717958647692f;
    }

    Wavetable::Wavetable()
    {
        for (int f = 0; f < kNumFrames; ++f)
        {
            const int numHarmonics = 1 + f * 3; // 1, 4, 7, ... 22: sine -> increasingly rich

            float peak = 0.0f;
            for (int i = 0; i < kTableSize; ++i)
            {
                const float phase = static_cast<float>(i) / static_cast<float>(kTableSize);
                float value = 0.0f;
                for (int h = 1; h <= numHarmonics; ++h)
                    value += std::sin(kTwoPi * static_cast<float>(h) * phase) / static_cast<float>(h);

                frames[static_cast<size_t>(f)][static_cast<size_t>(i)] = value;
                peak = std::max(peak, std::abs(value));
            }

            if (peak > 0.0f)
                for (auto& sample : frames[static_cast<size_t>(f)])
                    sample /= peak;
        }
    }

    float Wavetable::sampleFrame(int frameIndex, float phase) const
    {
        const float position = phase * static_cast<float>(kTableSize);
        const int i0 = static_cast<int>(position) % kTableSize;
        const int i1 = (i0 + 1) % kTableSize;
        const float frac = position - std::floor(position);

        const auto& table = frames[static_cast<size_t>(frameIndex)];
        return table[static_cast<size_t>(i0)] + (table[static_cast<size_t>(i1)] - table[static_cast<size_t>(i0)]) * frac;
    }

    float Wavetable::sample(float phase, float framePosition) const
    {
        framePosition = std::clamp(framePosition, 0.0f, 1.0f);
        phase -= std::floor(phase);

        const float framePos = framePosition * static_cast<float>(kNumFrames - 1);
        const int f0 = static_cast<int>(framePos);
        const int f1 = std::min(f0 + 1, kNumFrames - 1);
        const float frac = framePos - static_cast<float>(f0);

        return sampleFrame(f0, phase) + (sampleFrame(f1, phase) - sampleFrame(f0, phase)) * frac;
    }

    const Wavetable& getSharedWavetable()
    {
        static const Wavetable table;
        return table;
    }
}
