#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"
#include "md/dsp/Shaper.h"

/** TRX-SD: classic analog-style snare. SYN params: PTCH, DEC, RAMP, RDEC,
    STRT, NOIS, HARM, CLIP. Two detuned triangle oscillators (HARM blends in
    the second, higher one) with a pitch-drop sweep (RAMP/RDEC) form the
    tonal body; STRT delays the noise layer's onset slightly behind the
    tone's attack (a short pre-delay, in ms) before it decays over DEC/NOIS
    level; CLIP is a distortion stage. */
class TRXSDEngine : public md::MachineEngine
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
    md::PitchDropEnvelope rampEnv;
    md::AHDEnvelope bodyEnv, noiseEnv;

    double sampleRate = 44100.0;
    float ptchHz = 200.0f, dec = 0.15f, ramp = -6.0f, rdec = 0.03f;
    float strt = 3.0f, nois = 0.5f, harm = 0.4f, clip = 0.0f;

    float baseFreq = 200.0f;
    int strtSamples = 0, samplesSinceTrigger = 0;
    bool noiseStarted = false;
    float velocity = 1.0f;
    bool active = false;
};
