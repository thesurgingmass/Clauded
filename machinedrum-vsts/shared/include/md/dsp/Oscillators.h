#pragma once
#include <cmath>

namespace md
{
    constexpr float twoPi = 6.283185307179586f;

    /** Band-limit-free analog-style oscillator (drum synthesis doesn't need
        strict anti-aliasing the way a lead synth does; a touch of aliasing
        reads as "analog grit" here, matching the character of the original
        analog-modelled machines). */
    class AnalogOsc
    {
    public:
        void prepare(double sr) { sampleRate = sr; }
        void reset() { phase = 0.0f; }

        void setFrequency(float hz) { frequency = hz; }

        float sine()
        {
            float v = std::sin(phase);
            advance();
            return v;
        }

        float triangle()
        {
            float v = 2.0f * std::fabs(2.0f * (phase / twoPi) - 1.0f) - 1.0f;
            advance();
            return v;
        }

        float saw()
        {
            float v = 2.0f * (phase / twoPi) - 1.0f;
            advance();
            return v;
        }

        float square(float pulseWidth = 0.5f)
        {
            float v = (phase / twoPi) < pulseWidth ? 1.0f : -1.0f;
            advance();
            return v;
        }

    private:
        void advance()
        {
            phase += twoPi * frequency / (float) sampleRate;
            if (phase >= twoPi)
                phase -= twoPi;
        }

        double sampleRate = 44100.0;
        float phase = 0.0f;
        float frequency = 100.0f;
    };

    /** White noise generator (xorshift for speed/determinism, no <random> alloc). */
    class NoiseGen
    {
    public:
        float next()
        {
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            return ((float) (state & 0xFFFFFF) / (float) 0x800000) - 1.0f;
        }

    private:
        uint32_t state = 0x9F3B27A1u;
    };

    /** Single sine operator with phase-modulation input, the basic building
        block for the EFM1/EFM2 style FM machines. */
    class FMOperator
    {
    public:
        void prepare(double sr) { sampleRate = sr; }
        void reset() { phase = 0.0f; }
        void setFrequency(float hz) { frequency = hz; }

        float process(float phaseModIn = 0.0f)
        {
            float v = std::sin(phase + phaseModIn);
            phase += twoPi * frequency / (float) sampleRate;
            if (phase >= twoPi) phase -= twoPi;
            return v;
        }

    private:
        double sampleRate = 44100.0;
        float phase = 0.0f;
        float frequency = 100.0f;
    };
}
