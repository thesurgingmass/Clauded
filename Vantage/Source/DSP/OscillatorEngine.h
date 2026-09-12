#pragma once

#include <juce_core/juce_core.h>
#include "Constants.h"

namespace vantage
{
    /**
     * One oscillator slot. `model` picks the character:
     *  - Moog: band-limited saw with slow analog-style pitch drift.
     *  - Oberheim: two detuned band-limited saws (thick multi-osc unison).
     *  - Roland: band-limited saw with a subtle built-in chorus wobble.
     *  - Nord: clean, stable saw/square blend (no drift/wobble).
     *  - Wavetable: frame-interpolated lookup into the built-in wavetable bank.
     */
    class OscillatorEngine
    {
    public:
        void prepare(double newSampleRate);
        void reset();

        void setModel(OscillatorModel newModel) { model = newModel; }
        OscillatorModel getModel() const { return model; }

        void setFrequency(float frequencyHz);
        void setWavetablePosition(float position01) { wavetablePosition = juce::jlimit(0.0f, 1.0f, position01); }

        float renderSample();

    private:
        float renderMoog();
        float renderOberheim();
        float renderRoland();
        float renderNord();
        float renderWavetable();

        void advancePhase(float& phaseToAdvance, float increment) const;

        OscillatorModel model = OscillatorModel::Moog;
        double sampleRate = 44100.0;

        float phase = 0.0f;
        float phaseIncrement = 0.0f;
        float frequencyHz = 440.0f;
        float wavetablePosition = 0.0f;

        // Moog: slow random-walk pitch drift.
        juce::Random driftRandom;
        float driftTarget = 0.0f;
        float driftValue = 0.0f;
        int driftCounter = 0;

        // Oberheim: a second, slightly detuned saw.
        float secondPhase = 0.0f;

        // Roland: a slow built-in chorus/vibrato LFO.
        float chorusPhase = 0.0f;
    };
}
