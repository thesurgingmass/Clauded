#pragma once

#include <juce_audio_basics/juce_audio_basics.h>

namespace vantage
{
    /** A single sound that responds to every MIDI note and channel. */
    class SynthSound : public juce::SynthesiserSound
    {
    public:
        bool appliesToNote(int) override { return true; }
        bool appliesToChannel(int) override { return true; }
    };
}
