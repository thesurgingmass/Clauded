#pragma once
#include "md/MachineEngine.h"
#include "md/engines/ModalBank.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Filter.h"
#include "md/dsp/Envelope.h"
#include <array>

/** PI-SD: physically-informed snare. SYN params: PTCH, DEC, HARD, RING,
    TENS, RVOL, RDEC. A 6-mode resonator bank (membrane body, TENS scaling
    pitch, HARD shaping the excitation) is joined by RING, a separate
    resonant "shell" tone with its own level (RVOL) and decay (RDEC). */
class PISDEngine : public md::MachineEngine
{
public:
    void prepare(double sampleRate) override;
    void reset() override;
    void trigger(float velocity01, float pitchOffsetSemitones) override;
    void setSynParams(const std::vector<float>& v) override;
    float renderSample() override;
    bool isActive() const override { return active; }

private:
    static constexpr int numModes = 6;
    md::ModalBank<numModes> bank;
    md::NoiseGen noise;
    md::StateVariableFilter ringFilter;
    md::AHDEnvelope ringEnv;

    double sampleRate = 44100.0;
    float ptchHz = 190.0f, dec = 0.15f, hard = 0.5f, ring = 0.3f;
    float tens = 0.5f, rvol = 0.3f, rdec = 0.25f;

    int excitationSamples = 0, samplesSinceTrigger = 0;
    float velocity = 1.0f;
    bool active = false;
};
