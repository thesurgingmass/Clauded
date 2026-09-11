#include "md/MachineProcessor.h"
#include "PIMLEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "PI-ML";
    info.category = "PI";
    info.synParams = {
        { "SYN1", "PTCH", 100.0f, 1200.0f, 300.0f, "Hz" },
        { "SYN2", "DEC",  0.02f,  1.5f,    0.3f,   "s"  },
        { "SYN3", "HARD", 0.0f,   1.0f,    0.4f },
        { "SYN4", "TENS", 0.0f,   1.0f,    0.5f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<PIMLEngine>());
}
