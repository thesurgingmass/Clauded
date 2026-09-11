#include "md/MachineProcessor.h"
#include "TRXB2Engine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "TRX-B2";
    info.category = "TRX";
    info.synParams = {
        { "SYN1", "PTCH", 30.0f,   400.0f, 50.0f,  "Hz" },
        { "SYN2", "DEC",  0.02f,   3.0f,   0.5f,   "s"  },
        { "SYN3", "BUMP", 0.0f,    1.0f,   0.3f },
        { "SYN4", "BENV", 0.005f,  1.0f,   0.1f,   "s"  },
        { "SYN5", "SNAP", 0.0f,    1.0f,   0.3f },
        { "SYN6", "TONE", 0.0f,    1.0f,   0.0f },
        { "SYN7", "TUNE", -12.0f,  12.0f,  0.0f,   "st" },
        { "SYN8", "CLIP", 0.0f,    1.0f,   0.0f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<TRXB2Engine>());
}
