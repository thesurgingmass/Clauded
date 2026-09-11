#include "md/MachineProcessor.h"
#include "TRXCLEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "TRX-CL";
    info.category = "TRX";
    info.synParams = {
        { "SYN1", "PTCH", 800.0f,  4000.0f, 2200.0f, "Hz" },
        { "SYN2", "DEC",  0.01f,   0.3f,    0.05f,   "s"  },
        { "SYN3", "DUAL", 0.0f,    1.0f,    0.0f },
        { "SYN4", "ENH",  0.0f,    1.0f,    0.2f },
        { "SYN5", "TUNE", -12.0f,  12.0f,   5.0f,    "st" },
        { "SYN6", "CLIC", 0.0f,    1.0f,    0.3f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<TRXCLEngine>());
}
