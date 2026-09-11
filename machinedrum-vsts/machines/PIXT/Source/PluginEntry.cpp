#include "md/MachineProcessor.h"
#include "PIXTEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "PI-XT";
    info.category = "PI";
    info.synParams = {
        { "SYN1", "PTCH", 50.0f,   400.0f, 120.0f, "Hz" },
        { "SYN2", "DEC",  0.02f,   2.0f,   0.4f,   "s"  },
        { "SYN3", "HARD", 0.0f,    1.0f,   0.4f },
        { "SYN4", "HAMR", 0.0f,    1.0f,   0.6f },
        { "SYN5", "TUNE", -12.0f,  12.0f,  0.0f,   "st" },
        { "SYN6", "DAMP", 0.0f,    1.0f,   0.3f },
        { "SYN7", "SIZE", 0.0f,    1.0f,   0.4f },
        { "SYN8", "POS",  0.0f,    1.0f,   0.3f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<PIXTEngine>());
}
