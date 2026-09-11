#include "md/MachineProcessor.h"
#include "EFMCYEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "EFM-CY";
    info.category = "EFM";
    info.synParams = {
        { "SYN1", "PTCH", 600.0f,  6000.0f, 2200.0f, "Hz" },
        { "SYN2", "DEC",  0.05f,   3.0f,    1.2f,    "s"  },
        { "SYN3", "FB",   0.0f,    1.0f,    0.35f },
        { "SYN4", "HPF",  200.0f,  8000.0f, 2500.0f, "Hz" },
        { "SYN5", "MOD",  0.0f,    8.0f,    4.5f },
        { "SYN6", "MFRQ", 0.25f,   8.0f,    2.3f },
        { "SYN7", "MDEC", 0.005f,  1.5f,    0.3f,    "s"  },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<EFMCYEngine>());
}
