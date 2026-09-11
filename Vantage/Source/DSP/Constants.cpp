#include "Constants.h"

namespace vantage
{
    juce::StringArray oscillatorModelChoices()
    {
        return { "Moog", "Oberheim", "Roland", "Nord", "Wavetable" };
    }

    juce::StringArray filterModelChoices()
    {
        return { "Moog", "Oberheim", "Roland", "Nord" };
    }

    juce::StringArray oscillatorRoutingChoices()
    {
        return { "Filter 1", "Filter 2", "Both", "Bypass" };
    }

    juce::StringArray filterRoutingChoices()
    {
        return { "Series", "Parallel" };
    }

    juce::StringArray polyphonyModeChoices()
    {
        return { "Mono", "Poly" };
    }

    juce::StringArray lfoWaveformChoices()
    {
        return { "Sine", "Triangle", "Saw", "Square", "S&H" };
    }

    juce::StringArray modSourceChoices()
    {
        return { "None", "LFO 1", "LFO 2", "LFO 3", "LFO 4", "LFO 5", "LFO 6", "Env 3" };
    }

    juce::StringArray modDestinationChoices()
    {
        return {
            "None",
            "Osc 1 Level", "Osc 2 Level", "Osc 3 Level",
            "Osc 1 Pitch", "Osc 2 Pitch", "Osc 3 Pitch",
            "Osc 1 Pan", "Osc 2 Pan", "Osc 3 Pan",
            "Noise Level",
            "Filter 1 Cutoff", "Filter 1 Resonance",
            "Filter 2 Cutoff", "Filter 2 Resonance",
            "Amp Level",
            "LFO 1 Rate", "LFO 2 Rate", "LFO 3 Rate",
            "LFO 4 Rate", "LFO 5 Rate", "LFO 6 Rate"
        };
    }
}
