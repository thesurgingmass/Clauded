#pragma once
#include "md/MachineEngine.h"
#include "md/engines/ModalBank.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Filter.h"
#include "md/dsp/Envelope.h"
#include <array>

/** PI-RS: physically-informed rimshot. SYN params: PTCH, DEC, HARD, RING,
    RVOL, RDEC. A small, fast 4-mode resonator bank (a hard, short strike)
    plus a separate ringing "rim" tone (RVOL level, RDEC decay). */
class PIRSEngine : public md::MachineEngine
{
public:
    void prepare(double sampleRate) override;
    void reset() override;
    void trigger(float velocity01, float pitchOffsetSemitones) override;
    void setSynParams(const std::vector<float>& v) override;
    float renderSample() override;
    bool isActive() const override { return active; }

private:
    static constexpr int numModes = 4;
    md::ModalBank<numModes> bank;
    md::NoiseGen noise;
    md::StateVariableFilter ringFilter;
    md::AHDEnvelope ringEnv;

    double sampleRate = 44100.0;
    float ptchHz = 400.0f, dec = 0.05f, hard = 0.6f, ring = 0.4f, rvol = 0.4f, rdec = 0.15f;

    int excitationSamples = 0, samplesSinceTrigger = 0;
    float velocity = 1.0f;
    bool active = false;
};
