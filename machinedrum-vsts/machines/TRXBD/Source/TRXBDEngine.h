#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"
#include "md/dsp/Shaper.h"

/** TRX-BD: classic analog-style kick. SYN params: PTCH, DEC, RAMP, RDEC,
    HOLD, TICK, NOIS, DIRT, DIST. A pitch-enveloped triangle/sine body
    (RAMP/RDEC sweep into PTCH) is held at full level (HOLD) before decaying
    (DEC); TICK adds a very short attack transient, NOIS a low-level
    textural noise bed, DIRT a gentle analog-style saturation and DIST a
    harder clipping stage on top of that. */
class TRXBDEngine : public md::MachineEngine
{
public:
    void prepare(double sampleRate) override;
    void reset() override;
    void trigger(float velocity01, float pitchOffsetSemitones) override;
    void setSynParams(const std::vector<float>& v) override;
    float renderSample() override;
    bool isActive() const override { return active; }

private:
    md::AnalogOsc osc;
    md::NoiseGen noise;
    md::PitchDropEnvelope rampEnv;
    md::AHDEnvelope ampEnv, tickEnv;

    double sampleRate = 44100.0;
    float ptchHz = 55.0f, dec = 0.4f, ramp = -30.0f, rdec = 0.05f;
    float hold = 0.0f, tick = 0.3f, nois = 0.05f, dirt = 0.0f, dist = 0.0f;

    float baseFreq = 55.0f;
    float velocity = 1.0f;
    bool active = false;
};
