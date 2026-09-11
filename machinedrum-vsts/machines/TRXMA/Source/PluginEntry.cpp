#include "md/MachineProcessor.h"
#include "TRXMAEngine.h"

static md::MachineInfo makeInfo()
{
    md::MachineInfo info;
    info.machineName = "TRX-MA";
    info.category = "TRX";
    info.synParams = {
        { "SYN1", "ATT",  0.001f, 0.2f,    0.005f, "s" },
        { "SYN2", "SUS",  0.0f,   0.5f,    0.05f,  "s" },
        { "SYN3", "REV",  0.02f,  1.0f,    0.2f,   "s" },
        { "SYN4", "DAMP", 0.0f,   1.0f,    0.3f },
        { "SYN5", "RATL", 0.0f,   1.0f,    0.4f },
        { "SYN6", "RTYP", 0.0f,   1.0f,    0.5f },
        { "SYN7", "TONE", 1000.0f, 12000.0f, 6000.0f, "Hz" },
        { "SYN8", "HARD", 0.0f,   1.0f,    0.2f },
    };
    return info;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new md::MachineProcessor(makeInfo(), std::make_unique<TRXMAEngine>());
}
