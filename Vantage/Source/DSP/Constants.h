#pragma once

#include <juce_core/juce_core.h>

namespace vantage
{
    constexpr int kNumOscillators = 3;
    constexpr int kNumFilters = 2;
    constexpr int kNumEnvelopes = 3;
    constexpr int kNumLFOs = 6;
    constexpr int kNumModMatrixSlots = 16;
    constexpr int kMaxVoices = 16;

    /** Analog/digital modeling character shared by oscillators and filters. */
    enum class OscillatorModel
    {
        Moog,      // Ladder / transistor-style
        Oberheim,  // State-variable style
        Roland,    // OTA style
        Nord,      // Digital VA style
        Wavetable, // User-definable wavetable engine
        count
    };

    enum class FilterModel
    {
        Moog,
        Oberheim,
        Roland,
        Nord,
        count
    };

    enum class OscillatorRouting
    {
        Filter1,
        Filter2,
        Both,
        Bypass,
        count
    };

    enum class FilterRouting
    {
        Series,
        Parallel,
        count
    };

    enum class PolyphonyMode
    {
        Mono,
        Poly,
        count
    };

    enum class LFOWaveform
    {
        Sine,
        Triangle,
        Saw,
        Square,
        SampleAndHold,
        count
    };

    /** Modulation sources: the 6 free LFOs plus the freely-assignable ENV 3. */
    enum class ModSource
    {
        None,
        LFO1,
        LFO2,
        LFO3,
        LFO4,
        LFO5,
        LFO6,
        Env3,
        count
    };

    /**
     * Modulation destinations. This is deliberately structured as an enum so new
     * destinations can be added as later phases expose more assignable encoders,
     * without changing the matrix's storage/serialization format.
     */
    enum class ModDestination
    {
        None,
        Osc1Level,
        Osc2Level,
        Osc3Level,
        Osc1Pitch,
        Osc2Pitch,
        Osc3Pitch,
        Osc1Pan,
        Osc2Pan,
        Osc3Pan,
        NoiseLevel,
        Filter1Cutoff,
        Filter1Resonance,
        Filter2Cutoff,
        Filter2Resonance,
        AmpLevel,
        LFO1Rate,
        LFO2Rate,
        LFO3Rate,
        LFO4Rate,
        LFO5Rate,
        LFO6Rate,
        count
    };

    juce::StringArray oscillatorModelChoices();
    juce::StringArray filterModelChoices();
    juce::StringArray oscillatorRoutingChoices();
    juce::StringArray filterRoutingChoices();
    juce::StringArray polyphonyModeChoices();
    juce::StringArray lfoWaveformChoices();
    juce::StringArray modSourceChoices();
    juce::StringArray modDestinationChoices();
}
