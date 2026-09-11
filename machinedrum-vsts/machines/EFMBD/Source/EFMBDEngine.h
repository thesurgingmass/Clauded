#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"

/** EFM-BD: 2-operator FM kick. SYN params (hardware names): PTCH, DEC,
    RAMP, RDEC, MOD, MFRQ, MDEC, MFB.

    Signal path: a sine carrier at PTCH is phase-modulated by a sine
    modulator running at MFRQ x carrier frequency; MOD sets modulation
    index, decaying over MDEC. RAMP/RDEC add a separate pitch sweep on top
    of the carrier (bipolar: negative = classic downward pitch-drop thump,
    positive = upward chirp). MFB feeds a portion of the modulator's own
    last output back into its own phase, for the harder/noisier FM
    character self-feedback FM operators are known for. DEC is the overall
    body amplitude decay (independent of the shared AMP page decay, exactly
    as the hardware's SYN and AMP pages both contribute their own decay). */
class EFMBDEngine : public md::MachineEngine
{
public:
    void prepare(double sampleRate) override;
    void reset() override;
    void trigger(float velocity01, float pitchOffsetSemitones) override;
    void setSynParams(const std::vector<float>& v) override;
    float renderSample() override;
    bool isActive() const override { return active; }

private:
    md::FMOperator carrier, modulator;
    md::PitchDropEnvelope rampEnv;
    md::AHDEnvelope modEnv;
    md::AHDEnvelope bodyEnv;

    double sampleRate = 44100.0;
    float ptchHz = 55.0f, dec = 0.4f, ramp = -24.0f, rdec = 0.05f;
    float mod = 3.0f, mfrq = 1.5f, mdec = 0.08f, mfb = 0.0f;

    float baseFreq = 55.0f;
    float velocity = 1.0f;
    float lastModOut = 0.0f;
    bool active = false;
};
