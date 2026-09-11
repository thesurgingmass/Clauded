#include "md/MachineProcessor.h"
#include "PIMAEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "PI-MA";
    info.category = "PI";
    info.synParams = {
        { "SYN1", "GRNS", 0.0f, 1.0f, 0.5f },
        { "SYN2", "DEC",  0.02f, 1.5f, 0.3f, "s" },
        { "SYN3", "GLEN", 0.0f, 1.0f, 0.5f },
        { "SYN4", "SIZE", 0.0f, 1.0f, 0.5f },
        { "SYN5", "HARD", 0.0f, 1.0f, 0.4f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<PIMAEngine>());
}
