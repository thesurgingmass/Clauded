#pragma once

#include <juce_core/juce_core.h>
#include <array>
#include "Constants.h"

namespace vantage
{
    /**
     * One filter slot, stereo. `model` picks the topology:
     *  - Moog: 4-pole transistor-ladder cascade, symmetric tanh saturation
     *    per stage, resonance as global feedback (self-oscillates near max).
     *  - Oberheim: 2-pole (12dB/oct) zero-delay-feedback state-variable
     *    filter with a mild tanh "growl" on the output, SEM-style.
     *  - Roland: the same 4-pole ladder structure as Moog but with an
     *    asymmetric saturation curve and a steeper resonance ramp, evoking
     *    OTA-ladder ICs' more aggressive, "squelchy" character.
     *  - Nord: two cascaded clean (linear) zero-delay-feedback SVF stages
     *    for a precise 24dB/oct digital-VA response with no saturation.
     */
    class FilterEngine
    {
    public:
        void prepare(double newSampleRate);
        void reset();

        void setModel(FilterModel newModel) { model = newModel; }
        FilterModel getModel() const { return model; }

        void setCutoffHz(float cutoffHz);
        void setResonance(float resonance01);
        void setDrive(float drive01) { drive = juce::jlimit(0.0f, 1.0f, drive01); }

        float processSample(int channel, float input);

    private:
        float processMoog(int channel, float input);
        float processOberheim(int channel, float input);
        float processRoland(int channel, float input);
        float processNord(int channel, float input);

        // One cascaded zero-delay-feedback SVF stage (Andrew Simper's trapezoidal-
        // integrator form); returns the lowpass output and updates ic1/ic2 in place.
        float processSvfLowpass(float input, float g, float k, float& ic1, float& ic2) const;

        FilterModel model = FilterModel::Moog;
        double sampleRate = 44100.0;
        float drive = 0.0f;
        float cutoffHz = 8000.0f;
        float resonance01 = 0.0f;

        // Ladder coefficients (Moog, Roland): one-pole stage gain + feedback amount.
        float ladderG = 0.5f;
        float ladderFeedback = 0.0f;

        // SVF coefficients (Oberheim, Nord): prewarped gain + damping.
        float svfG = 0.5f;
        float svfK = 1.0f;

        // Per-channel state.
        std::array<std::array<float, 4>, 2> ladderStages {}; // [channel][stage]
        std::array<float, 2> svfIc1 {}, svfIc2 {};             // Oberheim
        std::array<float, 2> svfIc1B {}, svfIc2B {};           // Nord, 2nd cascaded stage
    };
}
