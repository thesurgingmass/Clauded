#include "md/MachineProcessor.h"
#include "PIBDEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "PI-BD";
    info.category = "PI";
    info.synParams = {
        { "SYN1", "PTCH", 30.0f,  300.0f, 60.0f,  "Hz" },
        { "SYN2", "DEC",  0.02f,  2.0f,   0.4f,   "s"  },
        { "SYN3", "HARD", 0.0f,   1.0f,   0.3f },
        { "SYN4", "HAMR", 0.0f,   1.0f,   0.6f },
        { "SYN5", "TENS", 0.0f,   1.0f,   0.5f },
        { "SYN6", "DAMP", 0.0f,   1.0f,   0.3f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<PIBDEngine>());
}
