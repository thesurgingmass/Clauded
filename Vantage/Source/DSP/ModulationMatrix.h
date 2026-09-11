#pragma once

#include <array>
#include "Constants.h"

namespace vantage
{
    struct ModMatrixSlot
    {
        ModSource source = ModSource::None;
        ModDestination destination = ModDestination::None;
        float depth = 0.0f; // bipolar, -1..1
    };

    /**
     * Slot-based modulation matrix: each of the fixed slots routes one source
     * (an LFO or ENV 3) to one destination at a depth. Every destination's
     * total modulation is the sum of depth * sourceValue across all slots
     * targeting it, so several sources can stack on the same destination.
     */
    class ModulationMatrix
    {
    public:
        void setSlot(int slotIndex, const ModMatrixSlot& slot);
        void setSourceValue(ModSource source, float value);

        /** Sum of depth * sourceValue for every slot routed to this destination. */
        float getModulation(ModDestination destination) const;

        void reset();

    private:
        std::array<ModMatrixSlot, kNumModMatrixSlots> slots {};
        std::array<float, static_cast<size_t>(ModSource::count)> sourceValues {};
    };
}
