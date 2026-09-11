#include "Drive.h"
#include <cmath>
#include <algorithm>

namespace fm
{
    void Drive::setAmount(float amount01)
    {
        amount01 = std::clamp(amount01, 0.0f, 1.0f);

        // 1x .. 12x pre-gain, mapped with a curve so the low end (subtle
        // warmth) has more usable range than the extreme end (fuzz).
        preGain = 1.0f + amount01 * amount01 * 11.0f;

        // tanh(preGain) at unity input gives us the peak we're compressing
        // down from; dividing it back out keeps perceived loudness roughly
        // constant as drive increases.
        makeupGain = preGain > 1.0f ? 1.0f / std::tanh(preGain) : 1.0f;
    }

    float Drive::processSample(float input) const
    {
        return std::tanh(input * preGain) * makeupGain;
    }
}
