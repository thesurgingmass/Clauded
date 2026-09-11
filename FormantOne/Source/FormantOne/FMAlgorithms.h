#pragma once

#include <array>
#include <string>
#include <vector>

namespace formantone
{
    /**
        FormantOne's 8-operator FM routing.

        Yamaha never published the FS1R's internal algorithm-routing
        diagrams in enough detail to reverse-engineer exactly (and its
        operators are also shared with the formant generator in ways that
        aren't publicly documented), so rather than guess at a fake "exact"
        table, this ships a **systematically generated** set: every one of
        the 22 ways to partition 8 operators into independent serial
        "stacks" (see `buildStackAlgorithm` in FMAlgorithms.cpp) — spanning
        fully serial (deep, clangorous FM) to fully parallel/additive
        (organ/bell-like) — plus two hand-built algorithms with
        cross-modulation (one modulator feeding two carriers, and vice
        versa) for variety. That's a faithful-in-spirit reconstruction of
        *what a Yamaha 8-op algorithm list is for*, not a bit-exact copy of
        the FS1R's own list.
    */
    constexpr int numOperators = 8;

    struct Connection
    {
        int source;
        int destination;
    };

    struct Algorithm
    {
        std::string name;
        std::vector<Connection> connections;
        std::array<bool, numOperators> isCarrier;
        std::array<int, numOperators> processOrder;
    };

    const std::vector<Algorithm>& getAlgorithms();
}
