#pragma once

#include <array>
#include <vector>

namespace fmtone
{
    /**
        FM TONE's operator routing.

        Operators are indexed C=0, A=1, B=2, D=3, matching the machine's own
        operator names. Each algorithm is a small modulation graph: a list of
        (source -> destination) phase-modulation connections, which operators
        sum directly to the audible output ("carriers"), and a fixed
        processing order that guarantees every operator is rendered only
        after everything that modulates it.

        Note: Elektron hasn't published exact internal wiring diagrams for
        FM TONE's 8 algorithms, so these are a faithful-in-spirit
        approximation — the same progression from fully serial (deep,
        clangorous FM) to fully parallel (additive, bell/organ-like) that
        the real machine's algorithm list walks through — rather than a
        bit-exact reconstruction.
    */
    enum OperatorIndex
    {
        opC = 0,
        opA = 1,
        opB = 2,
        opD = 3,
        numOperators = 4
    };

    struct Connection
    {
        int source;
        int destination;
    };

    struct Algorithm
    {
        const char* name;
        std::vector<Connection> connections;
        std::array<bool, numOperators> isCarrier;
        std::array<int, numOperators> processOrder;
    };

    inline const std::array<Algorithm, 8>& getAlgorithms()
    {
        static const std::array<Algorithm, 8> algorithms { {
            // 1: fully serial chain D -> B -> A -> C, C is the only carrier.
            { "1: D>B>A>C", { { opD, opB }, { opB, opA }, { opA, opC } },
              { true, false, false, false }, { opD, opB, opA, opC } },

            // 2: same chain, plus D also feeding C directly.
            { "2: D>B>A>C +D>C", { { opD, opB }, { opB, opA }, { opA, opC }, { opD, opC } },
              { true, false, false, false }, { opD, opB, opA, opC } },

            // 3: D and B both modulate A, A modulates C.
            { "3: D+B>A>C", { { opD, opA }, { opB, opA }, { opA, opC } },
              { true, false, false, false }, { opD, opB, opA, opC } },

            // 4: two independent 2-op stacks, B>A and D>C.
            { "4: B>A, D>C", { { opB, opA }, { opD, opC } },
              { false, true, false, false }, { opB, opD, opA, opC } },

            // 5: D>B>C chain, A is a bare carrier alongside C.
            { "5: D>B>C, A", { { opD, opB }, { opB, opC } },
              { true, true, false, false }, { opD, opB, opA, opC } },

            // 6: B>C and D>A, two more 2-op stacks with different pairing.
            { "6: B>C, D>A", { { opB, opC }, { opD, opA } },
              { false, true, false, false }, { opB, opD, opA, opC } },

            // 7: D>A only; A, B and C are all carriers.
            { "7: D>A, A+B+C", { { opD, opA } },
              { true, true, true, false }, { opD, opA, opB, opC } },

            // 8: fully parallel, no cross-modulation (additive). Operator C
            // can still self-feedback, handled inside FMOperator itself.
            { "8: C+A+B+D", {},
              { true, true, true, true }, { opC, opA, opB, opD } },
        } };
        return algorithms;
    }
}
