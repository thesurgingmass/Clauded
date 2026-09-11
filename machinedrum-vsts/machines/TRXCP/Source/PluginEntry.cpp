#include "md/MachineProcessor.h"
#include "TRXCPEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "TRX-CP";
    info.category = "TRX";
    info.synParams = {
        { "SYN1", "CLPY", 1.0f,    40.0f,   12.0f,  "ms" },
        { "SYN2", "TONE", 300.0f,  4000.0f, 1200.0f, "Hz" },
        { "SYN3", "HARD", 0.0f,    1.0f,    0.3f },
        { "SYN4", "RICH", 0.0f,    1.0f,    0.3f },
        { "SYN5", "RATE", 0.1f,    10.0f,   2.0f,   "Hz" },
        { "SYN6", "ROOM", 0.0f,    1.0f,    0.3f },
        { "SYN7", "RSIZ", 0.0f,    1.0f,    0.4f },
        { "SYN8", "RTUN", 200.0f,  8000.0f, 3000.0f, "Hz" },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<TRXCPEngine>());
}
