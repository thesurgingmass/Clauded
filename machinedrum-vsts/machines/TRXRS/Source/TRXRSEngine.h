#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"
#include "md/dsp/Shaper.h"

/** TRX-RS: minimal analog rimshot. SYN params: PTCH, DEC, DIST -- the
    simplest machine on the hardware. A short, hard-decaying tone burst with
    an optional distortion stage. */
class TRXRSEngine : public md::MachineEngine
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
    md::AHDEnvelope ampEnv;

    double sampleRate = 44100.0;
    float ptchHz = 500.0f, dec = 0.04f, dist = 0.0f;

    float baseFreq = 500.0f;
    float velocity = 1.0f;
    bool active = false;
};
