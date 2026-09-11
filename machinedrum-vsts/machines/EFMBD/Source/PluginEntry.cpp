#include "md/MachineProcessor.h"
#include "EFMBDEngine.h"

// SYN param ranges are reasonable engineering estimates for a musical
// result; the hardware's internal 0-127 scaling for these is not publicly
// documented. Names/order match the real EFM-BD SYN page exactly.
static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "EFM-BD";
    info.category = "EFM";
    info.synParams = {
        { "SYN1", "PTCH", 30.0f,   400.0f, 55.0f,  "Hz" },
        { "SYN2", "DEC",  0.005f,  3.0f,   0.4f,   "s"  },
        { "SYN3", "RAMP", -48.0f,  48.0f,  -24.0f, "st" },
        { "SYN4", "RDEC", 0.005f,  1.0f,   0.05f,  "s"  },
        { "SYN5", "MOD",  0.0f,    8.0f,   3.0f },
        { "SYN6", "MFRQ", 0.25f,   16.0f,  1.5f },
        { "SYN7", "MDEC", 0.005f,  1.0f,   0.08f,  "s"  },
        { "SYN8", "MFB",  0.0f,    1.0f,   0.0f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<EFMBDEngine>());
}
