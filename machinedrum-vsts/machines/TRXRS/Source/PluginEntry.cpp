#include "md/MachineProcessor.h"
#include "TRXRSEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "TRX-RS";
    info.category = "TRX";
    info.synParams = {
        { "SYN1", "PTCH", 150.0f,  1500.0f, 500.0f, "Hz" },
        { "SYN2", "DEC",  0.01f,   0.3f,    0.04f,  "s"  },
        { "SYN3", "DIST", 0.0f,    1.0f,    0.0f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<TRXRSEngine>());
}
