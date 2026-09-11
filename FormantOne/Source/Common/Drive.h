#pragma once

namespace fs
{
    /**
        Simple analog-style saturation stage for the output stage. Tanh
        soft-clipping with a level-compensated makeup gain so raising drive
        doesn't just get louder, it gets more saturated.
    */
    class Drive
    {
    public:
        void setAmount(float amount01);
        float processSample(float input) const;

    private:
        float preGain = 1.0f;
        float makeupGain = 1.0f;
    };
}
