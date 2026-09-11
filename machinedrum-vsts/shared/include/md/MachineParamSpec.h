#pragma once
#include <juce_core/juce_core.h>
#include <vector>

namespace md
{
    /** Describes one knob on the machine's SYN page. FLT/AMP page params are
        fixed across all machines and are added automatically by
        MachineProcessorBase -- only the machine-specific SYN knobs need to
        be declared per-machine, mirroring how the Machinedrum's SYN page is
        the only page whose layout changes per machine model. */
    struct SynParamSpec
    {
        juce::String id;      // parameter ID, e.g. "SYN1"
        juce::String name;    // short label shown on the knob, e.g. "TUNE"
        float minValue;
        float maxValue;
        float defaultValue;
        juce::String suffix = {};
    };

    struct MachineInfo
    {
        juce::String machineName;   // e.g. "BD ANALOG"
        juce::String category;      // e.g. "BD"
        std::vector<SynParamSpec> synParams;
    };
}
