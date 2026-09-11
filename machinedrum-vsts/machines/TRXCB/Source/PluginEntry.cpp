#include "md/MachineProcessor.h"
#include "TRXCBEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "TRX-CB";
    info.category = "TRX";
    info.synParams = {
        { "SYN1", "PTCH", 200.0f,  1200.0f, 540.0f, "Hz" },
        { "SYN2", "DEC",  0.02f,   1.5f,    0.3f,   "s"  },
        { "SYN3", "ENH",  0.0f,    1.0f,    0.3f },
        { "SYN4", "DAMP", 0.0f,    1.0f,    0.3f },
        { "SYN5", "TONE", 500.0f,  8000.0f, 4000.0f, "Hz" },
        { "SYN6", "BUMP", 0.0f,    1.0f,    0.2f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<TRXCBEngine>());
}
