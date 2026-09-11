#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "RateLevelEnvelope.h"

namespace fs
{
    /** Matches the filter type list on the real hardware this is modeled
        after: a 24dB/oct low pass (two cascaded 12dB stages), a 12dB low
        pass, a 12dB high pass, a band pass, and a band-eliminate (notch). */
    enum class FilterType
    {
        lpf24 = 0,
        lpf12,
        hpf12,
        bpf,
        bef
    };

    /**
        A resonant multimode filter with its own envelope and key-tracking.

        Built on a standard trapezoidally-integrated ("TPT") state-variable
        filter core, which yields low/band/high/notch outputs from the same
        two-integrator loop each sample — a well-documented, general-purpose
        DSP technique (not something specific to any manufacturer's
        hardware), picked here because it cleanly supports every filter type
        in the list above from one small building block, including cascading
        two low pass stages for the 24dB/oct mode.
    */
    class MultiModeFilter
    {
    public:
        void prepare(double newSampleRate);
        void reset();

        void noteOn();
        void noteOff();

        void setType(FilterType newType) { type = newType; }
        void setBaseCutoffHz(float hz) { baseCutoffHz = hz; }
        void setResonance(float q) { resonance = q; }
        void setEnvelopeAmount(float octaves) { envelopeAmount = octaves; }
        void setKeyTrackAmount(float amount01) { keyTrackAmount = amount01; }
        void setEnvelope(const RateLevelEnvelope::Parameters& params) { envelope.setParameters(params); }

        /** Call once per block after the note/key-track/cutoff params are set. */
        void updateCoefficients(float currentMidiNote);

        float processSample(float input);

    private:
        struct SvfStage
        {
            float ic1eq = 0.0f, ic2eq = 0.0f;
            float g = 0.0f, k = 1.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f;

            struct Outputs { float low, band, high, notch; };

            void reset() { ic1eq = ic2eq = 0.0f; }

            void setCoefficients(float cutoffHz, float q, double sampleRate)
            {
                g = std::tan(juce::MathConstants<float>::pi * cutoffHz / static_cast<float>(sampleRate));
                k = 1.0f / juce::jmax(0.05f, q);
                a1 = 1.0f / (1.0f + g * (g + k));
                a2 = g * a1;
                a3 = g * a2;
            }

            Outputs process(float input)
            {
                const float v3 = input - ic2eq;
                const float v1 = a1 * ic1eq + a2 * v3;
                const float v2 = ic2eq + a2 * ic1eq + a3 * v3;
                ic1eq = 2.0f * v1 - ic1eq;
                ic2eq = 2.0f * v2 - ic2eq;

                const float low = v2;
                const float band = v1;
                const float high = input - k * v1 - v2;
                const float notch = input - k * v1;
                return { low, band, high, notch };
            }
        };

        SvfStage stage1, stage2;
        RateLevelEnvelope envelope;

        double sampleRate = 44100.0;
        FilterType type = FilterType::lpf24;
        float baseCutoffHz = 4000.0f;
        float resonance = 0.707f;
        float envelopeAmount = 0.0f; // in octaves, applied to cutoff
        float keyTrackAmount = 0.0f; // 0 = none, 1 = full 1:1 tracking from C3 (note 48)
        float currentNote = 60.0f;
    };
}
