#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/FMVoice.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"

/** EFM-CB: FM cowbell. SYN params: PTCH, DEC, SNAP, FB, MOD, MFRQ, MDEC.
    A single FM carrier/modulator pair (with self-feedback FB for the
    metallic clank) forms the sustained tone; SNAP briefly adds a
    higher-ratio detuned copy of the carrier right at the attack for the
    classic double-tone cowbell "clack". */
class EFMCBEngine : public md::MachineEngine
{
public:
    void prepare(double sampleRate) override;
    void reset() override;
    void trigger(float velocity01, float pitchOffsetSemitones) override;
    void setSynParams(const std::vector<float>& v) override;
    float renderSample() override;
    bool isActive() const override { return active; }

private:
    md::FM2Op fm;
    md::FMOperator snapOsc;
    md::AHDEnvelope bodyEnv, modEnv, snapEnv;

    double sampleRate = 44100.0;
    float ptchHz = 540.0f, dec = 0.3f, snap = 0.5f, fb = 0.2f;
    float mod = 2.0f, mfrq = 1.48f, mdec = 0.1f;

    float baseFreq = 540.0f;
    float velocity = 1.0f;
    bool active = false;
};
