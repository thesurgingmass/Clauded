#include "ModulationMatrix.h"

namespace vantage
{
    void ModulationMatrix::setSlot(int slotIndex, const ModMatrixSlot& slot)
    {
        if (slotIndex >= 0 && slotIndex < kNumModMatrixSlots)
            slots[static_cast<size_t>(slotIndex)] = slot;
    }

    void ModulationMatrix::setSourceValue(ModSource source, float value)
    {
        sourceValues[static_cast<size_t>(source)] = value;
    }

    float ModulationMatrix::getModulation(ModDestination destination) const
    {
        float total = 0.0f;

        for (const auto& slot : slots)
            if (slot.destination == destination && slot.source != ModSource::None)
                total += slot.depth * sourceValues[static_cast<size_t>(slot.source)];

        return total;
    }

    void ModulationMatrix::reset()
    {
        sourceValues.fill(0.0f);
    }
}
