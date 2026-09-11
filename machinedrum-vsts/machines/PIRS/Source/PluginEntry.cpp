#include "md/MachineProcessor.h"
#include "PIRSEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "PI-RS";
    info.category = "PI";
    info.synParams = {
        { "SYN1", "PTCH", 150.0f, 1200.0f, 400.0f, "Hz" },
        { "SYN2", "DEC",  0.01f,  0.4f,    0.05f,  "s"  },
        { "SYN3", "HARD", 0.0f,   1.0f,    0.6f },
        { "SYN4", "RING", 0.0f,   1.0f,    0.4f },
        { "SYN5", "RVOL", 0.0f,   1.0f,    0.4f },
        { "SYN6", "RDEC", 0.01f,  1.0f,    0.15f,  "s"  },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<PIRSEngine>());
}
