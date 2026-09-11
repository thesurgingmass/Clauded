#include "md/MachineProcessor.h"
#include "md/engines/CymbalEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "PI-HH";
    info.category = "PI";
    info.synParams = {
        { "SYN1", "PTCH", 800.0f, 6000.0f, 2800.0f, "Hz" },
        { "SYN2", "DEC",  0.02f,  1.5f,    0.15f,   "s"  },
        { "SYN3", "CLSN", 0.0f,   1.0f,    0.5f },
        { "SYN4", "RING", 0.0f,   1.0f,    0.2f },
        { "SYN5", "AG",   0.0f,   1.0f,    0.5f },
        { "SYN6", "AU",   0.0f,   1.0f,    0.7f },
        { "SYN7", "BR",   0.0f,   1.0f,    0.4f },
        { "SYN8", "CLOS", 0.0f,   1.0f,    0.3f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<md::CymbalEngine>());
}
