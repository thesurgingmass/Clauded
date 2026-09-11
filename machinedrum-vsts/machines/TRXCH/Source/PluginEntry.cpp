#include "md/MachineProcessor.h"
#include "md/engines/HiHatEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "TRX-CH";
    info.category = "TRX";
    info.synParams = {
        { "SYN1", "GAP",  0.0f,    1.0f,    0.3f },
        { "SYN2", "DEC",  0.005f,  1.0f,    0.06f,  "s"  },
        { "SYN3", "HPF",  500.0f,  10000.0f, 3000.0f, "Hz" },
        { "SYN4", "LPF",  2000.0f, 16000.0f, 12000.0f, "Hz" },
        { "SYN5", "MTAL", 0.0f,    1.0f,    0.6f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<md::HiHatEngine>());
}
