#pragma once

#include <random>

namespace fs
{
    enum class LfoShape
    {
        sine = 0,
        triangle,
        saw,
        square,
        sampleAndHold
    };

    /**
        One modulation LFO per voice, retriggered on note-on — mirrors the
        single shared modulation LFO found on DX/FS-era Yamaha FM synths,
        with sends to pitch, amplitude, and filter cutoff (the voice decides
        how to apply each depth; this class just produces the -1..1
        waveform plus its own delay/fade-in).
    */
    class Lfo
    {
    public:
        void prepare(double newSampleRate) { sampleRate = newSampleRate; reset(); }
        void reset();

        /** Retriggers phase and the delay/fade-in countdown. */
        void noteOn() { reset(); }

        void setShape(LfoShape newShape) { shape = newShape; }
        void setRateHz(float hz) { rateHz = hz; }
        void setDelaySeconds(float seconds) { delaySeconds = seconds; }

        /** Returns the next -1..1 sample, already scaled by the delay/fade-in. */
        float getNextSample();

    private:
        double sampleRate = 44100.0;
        double phase = 0.0;
        double previousPhase = 0.0;
        double elapsedSeconds = 0.0;

        LfoShape shape = LfoShape::triangle;
        float rateHz = 4.0f;
        float delaySeconds = 0.0f;
        static constexpr float fadeInSeconds = 0.3f;

        float heldValue = 0.0f;
        std::minstd_rand rng { 0x51ed270bu };
        std::uniform_real_distribution<float> distribution { -1.0f, 1.0f };
    };
}
