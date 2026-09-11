#pragma once
#include "md/MachineEngine.h"
#include "md/engines/ModalBank.h"
#include "md/dsp/Oscillators.h"
#include <array>

/** PI-XT: physically-informed tom. SYN params: PTCH, DEC, HARD, HAMR, TUNE,
    DAMP, SIZE, POS. An 8-mode resonator bank models the drum head; SIZE
    scales the body's base frequency down as it grows, POS is the strike
    position (weights which modes ring out most, the classic
    physical-modelling "where you hit it" control), TUNE a fine offset. */
class PIXTEngine : public md::MachineEngine
{
public:
    void prepare(double sampleRate) override;
    void reset() override;
    void trigger(float velocity01, float pitchOffsetSemitones) override;
    void setSynParams(const std::vector<float>& v) override;
    float renderSample() override;
    bool isActive() const override { return active; }

private:
    static constexpr int numModes = 8;
    md::ModalBank<numModes> bank;
    md::NoiseGen noise;

    double sampleRate = 44100.0;
    float ptchHz = 120.0f, dec = 0.4f, hard = 0.4f, hamr = 0.6f;
    float tune = 0.0f, damp = 0.3f, size = 0.4f, pos = 0.3f;

    int excitationSamples = 0, samplesSinceTrigger = 0;
    float velocity = 1.0f;
    bool active = false;
};
