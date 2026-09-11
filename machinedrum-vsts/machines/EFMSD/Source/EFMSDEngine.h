#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/FMVoice.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"

/** EFM-SD: FM snare. SYN params: PTCH, DEC, NOIS, NDEC, MOD, MFRQ, MDEC, CLIC.
    An FM tone body (carrier/modulator, decaying over DEC/MDEC) is layered
    with an independent noise component (NOIS level, NDEC decay) for the
    snare's "snap", plus a short fixed-length click transient (CLIC level)
    at the attack. */
class EFMSDEngine : public md::MachineEngine
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
    md::NoiseGen noise;
    md::AHDEnvelope bodyEnv, noiseEnv, clickEnv, modEnv;

    double sampleRate = 44100.0;
    float ptchHz = 180.0f, dec = 0.15f, nois = 0.5f, ndec = 0.12f;
    float mod = 2.0f, mfrq = 2.0f, mdec = 0.05f, clic = 0.3f;

    float baseFreq = 180.0f;
    float velocity = 1.0f;
    bool active = false;
};
