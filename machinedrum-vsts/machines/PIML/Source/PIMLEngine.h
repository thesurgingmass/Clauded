#pragma once
#include "md/MachineEngine.h"
#include "md/engines/ModalBank.h"
#include "md/dsp/Oscillators.h"
#include <array>

/** PI-ML: physically-informed mallet/log-type percussion. SYN params:
    PTCH, DEC, HARD, TENS. The simplest PI machine: a small resonator bank
    struck by a HARD-shaped impulse, TENS scaling the body's base pitch. */
class PIMLEngine : public md::MachineEngine
{
public:
    void prepare(double sampleRate) override;
    void reset() override;
    void trigger(float velocity01, float pitchOffsetSemitones) override;
    void setSynParams(const std::vector<float>& v) override;
    float renderSample() override;
    bool isActive() const override { return active; }

private:
    static constexpr int numModes = 5;
    md::ModalBank<numModes> bank;
    md::NoiseGen noise;

    double sampleRate = 44100.0;
    float ptchHz = 300.0f, dec = 0.3f, hard = 0.4f, tens = 0.5f;

    int excitationSamples = 0, samplesSinceTrigger = 0;
    float velocity = 1.0f;
    bool active = false;
};
