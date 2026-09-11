#include "md/MachineProcessor.h"
#include "TRXBDEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "TRX-BD";
    info.category = "TRX";
    info.synParams = {
        { "SYN1", "PTCH", 30.0f,   400.0f, 55.0f,  "Hz" },
        { "SYN2", "DEC",  0.02f,   3.0f,   0.4f,   "s"  },
        { "SYN3", "RAMP", -48.0f,  48.0f,  -30.0f, "st" },
        { "SYN4", "RDEC", 0.005f,  1.0f,   0.05f,  "s"  },
        { "SYN5", "HOLD", 0.0f,    0.5f,   0.0f,   "s"  },
        { "SYN6", "TICK", 0.0f,    1.0f,   0.3f },
        { "SYN7", "NOIS", 0.0f,    1.0f,   0.05f },
        { "SYN8", "DIRT", 0.0f,    1.0f,   0.0f },
        { "SYN9", "DIST", 0.0f,    1.0f,   0.0f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<TRXBDEngine>());
}
