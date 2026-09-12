#pragma once

#include <array>
#include "Constants.h"
#include "ModulationMatrix.h"

namespace vantage
{
    /**
     * Plain-data per-block snapshot of every parameter a voice needs. Built
     * from the APVTS by Parameters.cpp and pushed into every active voice
     * once per block, keeping the DSP layer free of any JUCE-processor-state
     * dependency.
     */
    struct OscillatorParams
    {
        OscillatorModel model = OscillatorModel::Moog;
        float level = 0.0f;
        float coarseSemitones = 0.0f;
        float fineCents = 0.0f;
        float pan = 0.0f;
        float wavetablePosition = 0.0f;
        OscillatorRouting routing = OscillatorRouting::Both;
    };

    struct FilterParams
    {
        FilterModel model = FilterModel::Moog;
        float cutoffHz = 8000.0f;
        float resonance01 = 0.0f;
        float drive01 = 0.0f;
    };

    struct EnvelopeParams
    {
        float attackSeconds = 0.01f;
        float decaySeconds = 0.2f;
        float sustain01 = 0.7f;
        float releaseSeconds = 0.3f;
    };

    struct LFOParams
    {
        float rateHz = 1.0f;
        LFOWaveform waveform = LFOWaveform::Sine;
    };

    struct VoiceParameters
    {
        std::array<OscillatorParams, kNumOscillators> oscillators {};
        float noiseColor = 0.5f;
        float noiseLevel = 0.0f;

        std::array<FilterParams, kNumFilters> filters {};
        FilterRouting filterRouting = FilterRouting::Series;

        std::array<EnvelopeParams, kNumEnvelopes> envelopes {};
        float filterEnvAmount = 0.0f; // ENV 2 -> both filters' cutoff, in Hz at full depth

        std::array<LFOParams, kNumLFOs> lfos {};
        std::array<ModMatrixSlot, kNumModMatrixSlots> modMatrixSlots {};

        float masterGain = 0.8f;
        float glideTimeMs = 0.0f; // mono-mode portamento time between notes
    };
}
