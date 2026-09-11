#include "MultiModeFilter.h"
#include <cmath>

namespace fs
{
    void MultiModeFilter::prepare(double newSampleRate)
    {
        sampleRate = newSampleRate;
        envelope.setSampleRate(newSampleRate);
        reset();
    }

    void MultiModeFilter::reset()
    {
        stage1.reset();
        stage2.reset();
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

        // Envelope contributes up to +/- envelopeAmount octaves, key-tracking
        // shifts cutoff by the note's distance from C3 (note 48) scaled by
        // keyTrackAmount, both applied as octave offsets so they compose
        // musically across the frequency range.
        const float envelopeOctaves = envelope.getNextSample() * envelopeAmount;
        const float keyTrackOctaves = ((currentNote - 48.0f) / 12.0f) * keyTrackAmount;

        float cutoff = baseCutoffHz * std::pow(2.0f, envelopeOctaves + keyTrackOctaves);
        cutoff = juce::jlimit(20.0f, static_cast<float>(sampleRate) * 0.49f, cutoff);

        const float q = juce::jlimit(0.5f, 20.0f, resonance);
        stage1.setCoefficients(cutoff, q, sampleRate);
        stage2.setCoefficients(cutoff, q, sampleRate);
    }

    float MultiModeFilter::processSample(float input)
    {
        switch (type)
        {
            case FilterType::lpf24:
            {
                const auto o1 = stage1.process(input);
                const auto o2 = stage2.process(o1.low);
                return o2.low;
            }
            case FilterType::lpf12: return stage1.process(input).low;
            case FilterType::hpf12: return stage1.process(input).high;
            case FilterType::bpf:   return stage1.process(input).band;
            case FilterType::bef:   return stage1.process(input).notch;
        }
        return input;
    }
}
