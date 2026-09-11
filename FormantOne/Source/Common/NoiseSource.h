#pragma once

#include <random>

namespace fs
{
    /** A plain white-noise generator, one per voice — shared as the single
        "glottal" excitation source for all three formant generators, so
        their resonances color a common noise source rather than each
        drawing independent noise (matching how a real vocal tract has one
        noise source shaped by several parallel resonances). */
    class NoiseSource
    {
    public:
        void reset() { rng.seed(0x9e3779b9u); }
        float nextSample() { return distribution(rng); }

    private:
        std::minstd_rand rng { 0x9e3779b9u };
        std::uniform_real_distribution<float> distribution { -1.0f, 1.0f };
    };
}
