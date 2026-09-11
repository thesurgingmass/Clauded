#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"
#include "md/dsp/Shaper.h"

/** TRX-B2: second analog kick variant with an extra sub "bump". SYN
    params: PTCH, DEC, BUMP, BENV, SNAP, TONE, TUNE, CLIP. TONE morphs the
    main body oscillator from sine through triangle to a soft saw; TUNE is a
    fine-tune offset layered on top of PTCH; BUMP/BENV add an extra
    sub-octave sine "bump" with its own short decay; SNAP is an attack-noise
    transient; CLIP a distortion stage. */
class TRXB2Engine : public md::MachineEngine
{
public:
    void prepare(double sampleRate) override;
    void reset() override;
    void trigger(float velocity01, float pitchOffsetSemitones) override;
    void setSynParams(const std::vector<float>& v) override;
    float renderSample() override;
    bool isActive() const override { return active; }

private:
    md::AnalogOsc osc, bumpOsc;
    md::NoiseGen noise;
    md::AHDEnvelope ampEnv, bumpEnv, snapEnv;

    double sampleRate = 44100.0;
    float ptchHz = 50.0f, dec = 0.5f, bump = 0.3f, benv = 0.1f;
    float snap = 0.3f, tone = 0.0f, tune = 0.0f, clip = 0.0f;

    float baseFreq = 50.0f;
    float velocity = 1.0f;
    bool active = false;
};
