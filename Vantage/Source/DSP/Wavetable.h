#pragma once

#include <array>

namespace vantage
{
    /**
     * A small built-in bank of additive-synthesis wavetable frames, morphing
     * from a pure sine (frame 0) to an increasingly rich, saw-like spectrum
     * (the last frame). Stands in for user-loaded wavetables until file
     * loading is implemented; the sampling API is the same either way.
     *
     * Frames are shared (immutable, built once) rather than per-oscillator,
     * since every Wavetable-model oscillator reads the same bank.
     */
    class Wavetable
    {
    public:
        static constexpr int kNumFrames = 8;
        static constexpr int kTableSize = 2048;

        Wavetable();

        /** `phase` in [0, 1); `framePosition` in [0, 1] morphs across the bank. */
        float sample(float phase, float framePosition) const;

    private:
        float sampleFrame(int frameIndex, float phase) const;

        std::array<std::array<float, kTableSize>, kNumFrames> frames {};
    };

    const Wavetable& getSharedWavetable();
}
