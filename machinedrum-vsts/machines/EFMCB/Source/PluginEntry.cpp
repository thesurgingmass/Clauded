#include "md/MachineProcessor.h"
#include "EFMCBEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "EFM-CB";
    info.category = "EFM";
    info.synParams = {
        { "SYN1", "PTCH", 200.0f,  1200.0f, 540.0f, "Hz" },
        { "SYN2", "DEC",  0.02f,   1.5f,    0.3f,   "s"  },
        { "SYN3", "SNAP", 0.0f,    1.0f,    0.5f },
        { "SYN4", "FB",   0.0f,    1.0f,    0.2f },
        { "SYN5", "MOD",  0.0f,    8.0f,    2.0f },
        { "SYN6", "MFRQ", 0.25f,   4.0f,    1.48f },
        { "SYN7", "MDEC", 0.005f,  1.0f,    0.1f,   "s"  },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<EFMCBEngine>());
}
