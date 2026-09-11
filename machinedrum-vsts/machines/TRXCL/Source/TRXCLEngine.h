#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"
#include "md/dsp/Shaper.h"

/** TRX-CL: analog claves. SYN params: PTCH, DEC, DUAL, ENH, TUNE, CLIC. A
    short, hard sine "woodblock" tone (DEC is very short by nature) with an
    optional second detuned tone layer (DUAL, offset by TUNE semitones),
    ENH wavefolding in extra harmonics for a harder edge, and CLIC adding a
    short noise click at the attack. */
class TRXCLEngine : public md::MachineEngine
{
public:
    void prepare(double sampleRate) override;
    void reset() override;
    void trigger(float velocity01, float pitchOffsetSemitones) override;
    void setSynParams(const std::vector<float>& v) override;
    float renderSample() override;
    bool isActive() const override { return active; }

private:
    md::AnalogOsc osc1, osc2;
    md::NoiseGen noise;
    md::AHDEnvelope ampEnv, clickEnv;

    double sampleRate = 44100.0;
    float ptchHz = 2200.0f, dec = 0.05f, dual = 0.0f, enh = 0.2f, tune = 5.0f, clic = 0.3f;

    float baseFreq = 2200.0f;
    float velocity = 1.0f;
    bool active = false;
};
