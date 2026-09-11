#include "md/MachineProcessor.h"
#include "EFMXTEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "EFM-XT";
    info.category = "EFM";
    info.synParams = {
        { "SYN1", "PTCH", 40.0f,   600.0f, 120.0f, "Hz" },
        { "SYN2", "DEC",  0.005f,  2.0f,   0.3f,   "s"  },
        { "SYN3", "RAMP", -48.0f,  48.0f,  -12.0f, "st" },
        { "SYN4", "RDEC", 0.005f,  1.0f,   0.06f,  "s"  },
        { "SYN5", "MOD",  0.0f,    8.0f,   2.5f },
        { "SYN6", "MFRQ", 0.25f,   16.0f,  1.8f },
        { "SYN7", "NDEC", 0.005f,  1.0f,   0.1f,   "s"  },
        { "SYN8", "CLIC", 0.0f,    1.0f,   0.2f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<EFMXTEngine>());
}
