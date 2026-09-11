#include "md/MachineProcessor.h"
#include "md/engines/CymbalEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "PI-CC";
    info.category = "PI";
    info.synParams = {
        { "SYN1", "PTCH", 700.0f, 6000.0f, 2400.0f, "Hz" },
        { "SYN2", "DEC",  0.1f,   3.0f,    1.0f,    "s"  },
        { "SYN3", "HARD", 0.0f,   1.0f,    0.6f },
        { "SYN4", "RING", 0.0f,   1.0f,    0.3f },
        { "SYN5", "AG",   0.0f,   1.0f,    0.7f },
        { "SYN6", "AU",   0.0f,   1.0f,    0.8f },
        { "SYN7", "BR",   0.0f,   1.0f,    0.5f },
        { "SYN8", "GRAB", 0.0f,   1.0f,    0.0f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<md::CymbalEngine>());
}
