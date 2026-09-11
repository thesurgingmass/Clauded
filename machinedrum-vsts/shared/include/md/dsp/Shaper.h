#pragma once
#include <cmath>
#include <algorithm>

namespace md
{
    /** Soft-saturation stage used for the shared AMP page "OD" (overdrive)
        control, and internally by the Hard/Plastic/Powder machine variants. */
    inline float softClip(float x, float drive)
    {
        float d = 1.0f + drive * 9.0f;
        float y = std::tanh(x * d);
        return y;
    }

    /** Asymmetric wavefolder, used by the FM machines for extra harmonics. */
    inline float fold(float x)
    {
        while (x > 1.0f || x < -1.0f)
        {
            if (x > 1.0f)  x = 2.0f - x;
            if (x < -1.0f) x = -2.0f - x;
        }
        return x;
    }
}
