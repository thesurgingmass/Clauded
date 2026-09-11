#pragma once

#include <juce_dsp/juce_dsp.h>
#include "RateLevelEnvelope.h"

namespace fs
{
    /**
        One formant element — the building block of the "vocal" side of the
        engine, three of which make up a voice (matching the FS1R's three
        formant generators, after the classic F1/F2/F3 vocal-formant idea).

        This models a formant as a source/filter pair, which is a faithful
        reconstruction of the *concept* the real hardware's formant
        generator implements (a resonant peak, with its own level contour,
        excited by either the note's pitch or noise) rather than a
        reverse-engineered copy of Yamaha's internal circuit, which was
        never published:

        - **Voiced** excitation: PAF-style ("phase-aligned formant") synthesis
          — once per fundamental-pitch period, a short windowed burst of a
          sine wave at the formant frequency is emitted. The window's width
          (the "bandwidth" parameter, 0..1 as a fraction of the pitch
          period) sets how resonant vs. breathy the formant sounds.
        - **Unvoiced** excitation: the shared noise source run through a
          resonant bandpass tuned to the same formant frequency/bandwidth —
          the standard way to generate consonant/breath-like formant energy.

        `voicedUnvoiced` (0 = fully voiced, 1 = fully unvoiced) crossfades
        between the two, and the whole thing is shaped by its own
        `RateLevelEnvelope` so a formant's prominence can open/close across
        a note (a "vowel" that shifts shape mid-sustain).
    */
    class FormantGenerator
    {
    public:
        void prepare(double newSampleRate);
        void reset();

        void noteOn();
        void noteOff();

        void setFrequency(float hz) { frequencyHz = hz; }
        void setBandwidth(float widthFraction01);
        void setLevel(float newLevel) { level = newLevel; }
        void setVoicedUnvoiced(float amount01);
        void setEnvelope(const RateLevelEnvelope::Parameters& params) { envelope.setParameters(params); }

        /** Recomputes the unvoiced-path filter coefficients; call once per
            block after frequency/bandwidth are set for the current note. */
        void updateForBlock();

        bool isEnvelopeActive() const { return envelope.isActive(); }

        float renderSample(double fundamentalHz, float noiseSample);

    private:
        double sampleRate = 44100.0;

        double periodPhase = 0.0;
        double formantPhase = 0.0;

        float frequencyHz = 800.0f;
        float bandwidth = 0.2f;
        float level = 1.0f;
        float voicedUnvoiced = 0.0f;

        juce::dsp::IIR::Filter<float> noiseFilter;
        RateLevelEnvelope envelope;
    };
}
