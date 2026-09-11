#include "md/MachineProcessor.h"
#include "TRXSDEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "TRX-SD";
    info.category = "TRX";
    info.synParams = {
        { "SYN1", "PTCH", 80.0f,   600.0f, 200.0f, "Hz" },
        { "SYN2", "DEC",  0.02f,   1.0f,   0.15f,  "s"  },
        { "SYN3", "RAMP", -24.0f,  24.0f,  -6.0f,  "st" },
        { "SYN4", "RDEC", 0.005f,  0.5f,   0.03f,  "s"  },
        { "SYN5", "STRT", 0.0f,    20.0f,  3.0f,   "ms" },
        { "SYN6", "NOIS", 0.0f,    1.0f,   0.5f },
        { "SYN7", "HARM", 0.0f,    1.0f,   0.4f },
        { "SYN8", "CLIP", 0.0f,    1.0f,   0.0f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<TRXSDEngine>());
}
