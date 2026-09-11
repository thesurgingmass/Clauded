#include "MultiModeFilter.h"
#include <cmath>

namespace fm
{
    void MultiModeFilter::prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
        filter.prepare(spec);
        envelope.setSampleRate(spec.sampleRate);
        reset();
    }

    void MultiModeFilter::reset()
    {
        filter.reset();
        envelope.reset();
    }

    void MultiModeFilter::noteOn()
    {
        envelope.noteOn();
    }

    void MultiModeFilter::noteOff()
    {
        envelope.noteOff();
    }

    void MultiModeFilter::updateCoefficients(float currentMidiNote)
    {
        currentNote = currentMidiNote;

        switch (type)
        {
            case FilterType::lowPass:  filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass); break;
            case FilterType::highPass: filter.setType(juce::dsp::StateVariableTPTFilterType::highpass); break;
            case FilterType::bandPass: filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass); break;
        }

        // Envelope contributes up to +/- envelopeAmount octaves, key-tracking
        // shifts cutoff by the note's distance from C3 (note 48) scaled by
        // keyTrackAmount, both applied as octave offsets so they compose
        // musically across the frequency range.
        const float envelopeOctaves = envelope.getNextSample() * envelopeAmount;
        const float keyTrackOctaves = ((currentNote - 48.0f) / 12.0f) * keyTrackAmount;

        float cutoff = baseCutoffHz * std::pow(2.0f, envelopeOctaves + keyTrackOctaves);
        cutoff = juce::jlimit(20.0f, static_cast<float>(sampleRate) * 0.49f, cutoff);

        filter.setCutoffFrequency(cutoff);
        filter.setResonance(juce::jlimit(0.1f, 20.0f, resonance));
    }

    float MultiModeFilter::processSample(float input)
    {
        return filter.processSample(0, input);
    }
}
