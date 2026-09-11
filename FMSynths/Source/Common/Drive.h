#pragma once

namespace fm
{
    /**
        Simple analog-style saturation stage, matching the "overdrive" control
        found on the amp page of both source machines. Tanh soft-clipping with
        a level-compensated makeup gain so raising drive doesn't just get
        louder, it gets more saturated.
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
