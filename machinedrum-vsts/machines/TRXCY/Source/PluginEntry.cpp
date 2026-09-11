#include "md/MachineProcessor.h"
#include "TRXCYEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "TRX-CY";
    info.category = "TRX";
    info.synParams = {
        { "SYN1", "RICH", 0.0f,    1.0f,     0.6f },
        { "SYN2", "DEC",  0.05f,   3.0f,     1.2f,    "s"  },
        { "SYN3", "TOP",  0.0f,    1.0f,     0.4f },
        { "SYN4", "TTUN", 1000.0f, 14000.0f, 8000.0f, "Hz" },
        { "SYN5", "SIZE", 0.0f,    1.0f,     0.4f },
        { "SYN6", "PEAK", 0.0f,    1.0f,     0.2f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<TRXCYEngine>());
}
