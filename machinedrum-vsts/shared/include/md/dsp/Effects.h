#pragma once
#include "Filter.h"
#include "Oscillators.h"
#include <cmath>
#include <vector>
#include <algorithm>

namespace md
{
    /** The Machinedrum's per-track filter: a lowpass and a highpass stage
        sharing a base cutoff (FLTF) with a gap width (FLTW) that separates
        their two corner frequencies -- with width at 0 both stages sit on
        top of each other, and as width grows a "gap" of attenuated
        frequencies opens up between the two passbands. Both stages share a
        quality/resonance amount (FLTQ). */
    class GapFilter
    {
    public:
        void prepare(double sr) { lp.prepare(sr); hp.prepare(sr); }
        void reset() { lp.reset(); hp.reset(); }

        void setParams(float baseHz, float widthHz, float q01)
        {
            float half = std::max(0.0f, widthHz) * 0.5f;
            lp.setType(FilterType::LowPass);
            hp.setType(FilterType::HighPass);
            lp.setParams(std::clamp(baseHz + half, 20.0f, 20000.0f), q01);
            hp.setParams(std::clamp(baseHz - half, 20.0f, 20000.0f), q01);
        }

        float process(float in) { return lp.process(in) + hp.process(in); }

    private:
        StateVariableFilter lp, hp;
    };

    /** Free-running tremolo: continuously modulates amplitude at AMF Hz by
        AMD depth, independent of note triggering (as on the hardware). */
    class Tremolo
    {
    public:
        void prepare(double sr) { sampleRate = sr; }
        void reset() { phase = 0.0f; }

        float process(float in, float depth01, float freqHz)
        {
            phase += twoPi * freqHz / (float) sampleRate;
            if (phase >= twoPi) phase -= twoPi;
            float mod = 1.0f - depth01 * 0.5f * (1.0f - std::sin(phase));
            return in * mod;
        }

    private:
        double sampleRate = 44100.0;
        float phase = 0.0f;
    };

    /** Sample-rate reduction (bitcrusher style sample-and-hold). amount01=0
        is full rate (no effect), amount01=1 is heavily downsampled. */
    class SampleRateReducer
    {
    public:
        void reset() { counter = 0; held = 0.0f; }

        float process(float in, float amount01)
        {
            int holdSamples = 1 + (int) (amount01 * amount01 * 48.0f);
            if (counter <= 0)
            {
                held = in;
                counter = holdSamples;
            }
            --counter;
            return held;
        }

    private:
        int counter = 0;
        float held = 0.0f;
    };

    /** Simple feedback delay line, standing in for a send to the hardware's
        shared Rhythm Echo delay (which has no equivalent inside a
        standalone plugin instance). Fixed internal time/feedback; the
        per-track DEL knob only controls send amount, matching the real
        control surface. */
    class SimpleDelay
    {
    public:
        void prepare(double sr)
        {
            sampleRate = sr;
            buffer.assign((size_t) (sr * 1.2), 0.0f);
            writePos = 0;
        }

        void reset() { std::fill(buffer.begin(), buffer.end(), 0.0f); writePos = 0; }

        float process(float in)
        {
            int delaySamples = (int) (sampleRate * 0.375);
            size_t readPos = (writePos + buffer.size() - (size_t) delaySamples) % buffer.size();
            float delayed = buffer[readPos];
            buffer[writePos] = in + delayed * 0.35f;
            writePos = (writePos + 1) % buffer.size();
            return delayed;
        }

    private:
        double sampleRate = 44100.0;
        std::vector<float> buffer;
        size_t writePos = 0;
    };

    inline void equalPowerPan(float in, float pan01Bipolar, float& outL, float& outR)
    {
        float p = std::clamp(pan01Bipolar, -1.0f, 1.0f);
        float angle = (p + 1.0f) * (float) M_PI / 4.0f;
        outL = in * std::cos(angle);
        outR = in * std::sin(angle);
    }
}
