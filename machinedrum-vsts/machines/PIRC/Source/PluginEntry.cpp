#include "md/MachineProcessor.h"
#include "md/engines/CymbalEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "PI-RC";
    info.category = "PI";
    info.synParams = {
        { "SYN1", "PTCH", 600.0f, 5000.0f, 1800.0f, "Hz" },
        { "SYN2", "DEC",  0.1f,   4.0f,    1.5f,    "s"  },
        { "SYN3", "HARD", 0.0f,   1.0f,    0.4f },
        { "SYN4", "RING", 0.0f,   1.0f,    0.4f },
        { "SYN5", "AG",   0.0f,   1.0f,    0.5f },
        { "SYN6", "AU",   0.0f,   1.0f,    0.7f },
        { "SYN7", "BR",   0.0f,   1.0f,    0.3f },
        { "SYN8", "GRAB", 0.0f,   1.0f,    0.0f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<md::CymbalEngine>());
}
