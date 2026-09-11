#include "md/MachineProcessor.h"
#include "EFMSDEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "EFM-SD";
    info.category = "EFM";
    info.synParams = {
        { "SYN1", "PTCH", 60.0f,   1000.0f, 180.0f, "Hz" },
        { "SYN2", "DEC",  0.005f,  2.0f,    0.15f,  "s"  },
        { "SYN3", "NOIS", 0.0f,    1.0f,    0.5f },
        { "SYN4", "NDEC", 0.005f,  1.0f,    0.12f,  "s"  },
        { "SYN5", "MOD",  0.0f,    8.0f,    2.0f },
        { "SYN6", "MFRQ", 0.25f,   16.0f,   2.0f },
        { "SYN7", "MDEC", 0.005f,  1.0f,    0.05f,  "s"  },
        { "SYN8", "CLIC", 0.0f,    1.0f,    0.3f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<EFMSDEngine>());
}
