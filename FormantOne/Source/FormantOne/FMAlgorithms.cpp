#include "FMAlgorithms.h"

namespace formantone
{
    namespace
    {
        std::array<int, numOperators> descendingProcessOrder()
        {
            return { 7, 6, 5, 4, 3, 2, 1, 0 };
        }

        /**
            Builds one algorithm from a partition of the 8 operators into
            serial "stacks" (e.g. {4,4} = two independent 4-operator serial
            chains summed together; {1,1,1,1,1,1,1,1} = fully parallel/
            additive). Operator indices are assigned sequentially starting
            from 0 as each stack is consumed; within a stack the
            highest-indexed operator modulates the next one down, cascading
            to the lowest index in that stack, which becomes that stack's
            carrier.

            Every connection built this way has source > destination, so a
            single descending process order (7,6,5,...,0) is always valid —
            every modulator is rendered before anything it modulates.
        */
        Algorithm buildStackAlgorithm(const std::vector<int>& stackSizes)
        {
            Algorithm algo;
            algo.isCarrier.fill(false);
            algo.processOrder = descendingProcessOrder();

            std::string name;
            int nextIndex = 0;
            for (size_t s = 0; s < stackSizes.size(); ++s)
            {
                const int size = stackSizes[s];
                const int lowIndex = nextIndex;
                const int highIndex = nextIndex + size - 1;

                for (int idx = highIndex; idx > lowIndex; --idx)
                    algo.connections.push_back({ idx, idx - 1 });

                algo.isCarrier[static_cast<size_t>(lowIndex)] = true;

                if (s > 0)
                    name += " + ";
                for (int idx = highIndex; idx >= lowIndex; --idx)
                {
                    name += std::to_string(idx + 1);
                    if (idx > lowIndex)
                        name += ">";
                }

                nextIndex += size;
            }

            algo.name = name;
            return algo;
        }

        Algorithm buildCrossModAlgorithmA()
        {
            // Op 7 and Op 8 both modulate Op 1; Ops 2-6 are five bare
            // parallel carriers alongside it.
            Algorithm algo;
            algo.isCarrier = { true, true, true, true, true, true, false, false };
            algo.processOrder = descendingProcessOrder();
            algo.connections = { { 7, 0 }, { 6, 0 } };
            algo.name = "7+8>1, 2+3+4+5+6";
            return algo;
        }

        Algorithm buildCrossModAlgorithmB()
        {
            // Op 8 modulates both Op 1 and Op 2 (two carriers sharing one
            // modulator); Op 7>Op 6>Op 5 chain; Op 4>Op 3 chain.
            Algorithm algo;
            algo.isCarrier = { true, true, true, false, true, false, false, false };
            algo.processOrder = descendingProcessOrder();
            algo.connections = { { 7, 0 }, { 7, 1 }, { 6, 5 }, { 5, 4 }, { 3, 2 } };
            algo.name = "8>1+2, 7>6>5, 4>3";
            return algo;
        }
    }

    const std::vector<Algorithm>& getAlgorithms()
    {
        static const std::vector<Algorithm> algorithms = [] {
            std::vector<Algorithm> result;

            // All 22 integer partitions of 8, from fully serial to fully
            // parallel/additive.
            const std::vector<std::vector<int>> partitions {
                { 8 },
                { 7, 1 }, { 6, 2 }, { 6, 1, 1 }, { 5, 3 }, { 5, 2, 1 }, { 5, 1, 1, 1 },
                { 4, 4 }, { 4, 3, 1 }, { 4, 2, 2 }, { 4, 2, 1, 1 }, { 4, 1, 1, 1, 1 },
                { 3, 3, 2 }, { 3, 3, 1, 1 }, { 3, 2, 2, 1 }, { 3, 2, 1, 1, 1 }, { 3, 1, 1, 1, 1, 1 },
                { 2, 2, 2, 2 }, { 2, 2, 2, 1, 1 }, { 2, 2, 1, 1, 1, 1 }, { 2, 1, 1, 1, 1, 1, 1 },
                { 1, 1, 1, 1, 1, 1, 1, 1 },
            };

            for (const auto& partition : partitions)
                result.push_back(buildStackAlgorithm(partition));

            result.push_back(buildCrossModAlgorithmA());
            result.push_back(buildCrossModAlgorithmB());

            for (size_t i = 0; i < result.size(); ++i)
                result[i].name = std::to_string(i + 1) + ": " + result[i].name;

            return result;
        }();
        return algorithms;
    }
}
