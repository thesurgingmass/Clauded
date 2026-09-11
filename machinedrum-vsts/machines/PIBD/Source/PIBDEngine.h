#pragma once
#include "md/MachineEngine.h"
#include "md/engines/ModalBank.h"
#include "md/dsp/Oscillators.h"
#include <array>

/** PI-BD: physically-informed kick. SYN params: PTCH, DEC, HARD, HAMR,
    TENS, DAMP. A short noise/impulse excitation (HARD sets its brightness
    via burst length, HAMR its level) strikes an 8-mode resonator bank tuned
    to idealized circular-membrane mode ratios; TENS scales the membrane's
    base pitch (tension), DAMP shapes how much faster the higher modes die
    out relative to the fundamental. HARD/HAMR/AG/AU/BR-style parameter
    semantics are not publicly documented at this level of detail -- this
    mapping is a physically-plausible approximation, not a hardware-verified
    one. */
class PIBDEngine : public md::MachineEngine
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
    float ptchHz = 60.0f, dec = 0.4f, hard = 0.3f, hamr = 0.6f, tens = 0.5f, damp = 0.3f;

    int excitationSamples = 0, samplesSinceTrigger = 0;
    float velocity = 1.0f;
    bool active = false;
};
