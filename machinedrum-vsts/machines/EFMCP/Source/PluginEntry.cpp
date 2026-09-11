#include "md/MachineProcessor.h"
#include "EFMCPEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "EFM-CP";
    info.category = "EFM";
    info.synParams = {
        { "SYN1", "PTCH", 200.0f,  2000.0f, 900.0f, "Hz" },
        { "SYN2", "DEC",  0.005f,  1.0f,    0.1f,   "s"  },
        { "SYN3", "CLPS", 1.0f,    40.0f,   12.0f,  "ms" },
        { "SYN4", "CDEC", 0.02f,   1.5f,    0.25f,  "s"  },
        { "SYN5", "MOD",  0.0f,    8.0f,    1.0f },
        { "SYN6", "MFRQ", 0.25f,   16.0f,   3.0f },
        { "SYN7", "MDEC", 0.005f,  1.0f,    0.05f,  "s"  },
        { "SYN8", "HPF",  100.0f,  4000.0f, 800.0f, "Hz" },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<EFMCPEngine>());
}
