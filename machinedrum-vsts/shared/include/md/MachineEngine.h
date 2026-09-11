#pragma once
#include <vector>

namespace md
{
    /** A machine engine implements only the SYN-page synthesis (oscillators,
        noise, its own internal envelopes/pitch sweep). The shared FLT and
        AMP stages -- identical signal-flow across every Machinedrum machine
        -- are applied generically by MachineProcessorBase, exactly mirroring
        the hardware's SYN -> FLT -> AMP per-track signal path. */
    class MachineEngine
    {
    public:
        virtual ~MachineEngine() = default;

        virtual void prepare(double sampleRate) = 0;
        virtual void reset() = 0;

        /** velocity01 is 0..1. pitchOffsetSemitones is 0 in normal drum-trigger
            mode, or the keyboard offset from the mapping root note when the
            KYBD toggle is engaged. */
        virtual void trigger(float velocity01, float pitchOffsetSemitones) = 0;

        /** Called once per sample block boundary is unnecessary; called every
            sample so an engine may read current knob values without a stale
            copy -- cheap since these are plain floats, not smoothed here. */
        virtual void setSynParams(const std::vector<float>& values) = 0;

        /** Renders and returns one raw synthesis sample. */
        virtual float renderSample() = 0;

        /** Whether the engine still has audible internal envelope activity. */
        virtual bool isActive() const = 0;
    };
}
