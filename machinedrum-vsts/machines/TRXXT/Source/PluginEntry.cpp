#include "md/MachineProcessor.h"
#include "md/engines/TomEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "TRX-XT";
    info.category = "TRX";
    info.synParams = {
        { "SYN1", "PTCH", 50.0f,   500.0f, 150.0f, "Hz" },
        { "SYN2", "DEC",  0.02f,   2.0f,   0.3f,   "s"  },
        { "SYN3", "RAMP", -36.0f,  36.0f,  -12.0f, "st" },
        { "SYN4", "RDEC", 0.005f,  1.0f,   0.05f,  "s"  },
        { "SYN5", "DAMP", 0.0f,    1.0f,   0.3f },
        { "SYN6", "DIST", 0.0f,    1.0f,   0.0f },
        { "SYN7", "DTYP", 0.0f,    2.0f,   0.0f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<md::TomEngine>(0.2f));
}
