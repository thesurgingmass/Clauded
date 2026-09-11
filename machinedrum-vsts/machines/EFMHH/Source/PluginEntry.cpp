#include "md/MachineProcessor.h"
#include "EFMHHEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "EFM-HH";
    info.category = "EFM";
    info.synParams = {
        { "SYN1", "PTCH", 800.0f,  8000.0f, 3000.0f, "Hz" },
        { "SYN2", "DEC",  0.02f,   1.5f,    0.2f,    "s"  },
        { "SYN3", "TREM", 0.0f,    1.0f,    0.3f },
        { "SYN4", "TFRQ", 2.0f,    120.0f,  40.0f,   "Hz" },
        { "SYN5", "MOD",  0.0f,    8.0f,    4.0f },
        { "SYN6", "MFRQ", 0.25f,   8.0f,    3.7f },
        { "SYN7", "MDEC", 0.005f,  1.0f,    0.06f,   "s"  },
        { "SYN8", "FB",   0.0f,    1.0f,    0.3f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<EFMHHEngine>());
}
