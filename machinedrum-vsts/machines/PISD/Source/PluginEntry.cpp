#include "md/MachineProcessor.h"
#include "PISDEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "PI-SD";
    info.category = "PI";
    info.synParams = {
        { "SYN1", "PTCH", 100.0f, 500.0f, 190.0f, "Hz" },
        { "SYN2", "DEC",  0.02f,  1.0f,   0.15f,  "s"  },
        { "SYN3", "HARD", 0.0f,   1.0f,   0.5f },
        { "SYN4", "RING", 0.0f,   1.0f,   0.3f },
        { "SYN5", "TENS", 0.0f,   1.0f,   0.5f },
        { "SYN6", "RVOL", 0.0f,   1.0f,   0.3f },
        { "SYN7", "RDEC", 0.02f,  1.0f,   0.25f,  "s"  },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<PISDEngine>());
}
