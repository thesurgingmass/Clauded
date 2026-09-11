#include "md/MachineProcessor.h"
#include "EFMRSEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "EFM-RS";
    info.category = "EFM";
    info.synParams = {
        { "SYN1", "PTCH",  100.0f, 1200.0f, 400.0f, "Hz" },
        { "SYN2", "DEC",   0.005f, 1.0f,    0.08f,  "s"  },
        { "SYN3", "NOISE", 0.0f,   1.0f,    0.4f },
        { "SYN4", "NDEC",  0.005f, 1.0f,    0.05f,  "s"  },
        { "SYN5", "MOD",   0.0f,   8.0f,    3.0f },
        { "SYN6", "MFRQ",  0.25f,  16.0f,   2.5f },
        { "SYN7", "MDEC",  0.005f, 1.0f,    0.03f,  "s"  },
        { "SYN8", "HPF",   100.0f, 6000.0f, 1500.0f, "Hz" },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<EFMRSEngine>());
}
