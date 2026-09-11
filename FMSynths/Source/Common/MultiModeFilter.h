#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>

namespace fm
{
    enum class FilterType
    {
        lowPass = 0,
        highPass,
        bandPass
    };

    /**
        A resonant multimode filter with its own envelope and key-tracking,
        the same shape as the FLTR page shared by both the FM TONE and FM
        DRUM voices this project models: state-variable filter, selectable
        LP/HP/BP, an envelope that can push cutoff up or down, and
        key-tracking that scales cutoff with the played note.
    */
    class MultiModeFilter
    {
    public:
        void prepare(const juce::dsp::ProcessSpec& spec);
        void reset();

        void noteOn();
        void noteOff();

        void setType(FilterType newType) { type = newType; }
        void setBaseCutoffHz(float hz) { baseCutoffHz = hz; }
        void setResonance(float q) { resonance = q; }
        void setEnvelopeAmount(float semitonesOrOctaves) { envelopeAmount = semitonesOrOctaves; }
        void setKeyTrackAmount(float amount01) { keyTrackAmount = amount01; }
        void setEnvelope(const juce::ADSR::Parameters& params) { envelope.setParameters(params); }

        /** Call once per block after the note/key-track/cutoff params are set. */
        void updateCoefficients(float currentMidiNote);

        float processSample(float input);

    private:
        juce::dsp::StateVariableTPTFilter<float> filter;
        juce::ADSR envelope;

        double sampleRate = 44100.0;
        FilterType type = FilterType::lowPass;
        float baseCutoffHz = 4000.0f;
        float resonance = 0.707f;
        float envelopeAmount = 0.0f; // in octaves, applied to cutoff
        float keyTrackAmount = 0.0f; // 0 = none, 1 = full 1:1 tracking from C3 (note 48)
        float currentNote = 60.0f;
    };
}
