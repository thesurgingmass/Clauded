#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

namespace formantone
{
    /** A single sound that responds to every MIDI channel and note. */
    class FormantOneSound : public juce::SynthesiserSound
    {
    public:
        bool appliesToNote(int) override { return true; }
        bool appliesToChannel(int) override { return true; }
    };
}
