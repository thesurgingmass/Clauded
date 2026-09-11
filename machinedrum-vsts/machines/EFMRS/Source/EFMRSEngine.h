#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/FMVoice.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"
#include "md/dsp/Filter.h"

/** EFM-RS: FM rimshot. SYN params: PTCH, DEC, NOISE, NDEC, MOD, MFRQ, MDEC,
    HPF. Like EFM-SD's tone+noise layering, but shaped short/hard for a
    rimshot, with an internal highpass (HPF) on the noise layer instead of a
    separate click. */
class EFMRSEngine : public md::MachineEngine
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
    md::AHDEnvelope bodyEnv, noiseEnv, modEnv;
    md::StateVariableFilter hpf;

    double sampleRate = 44100.0;
    float ptchHz = 400.0f, dec = 0.08f, noiseAmt = 0.4f, ndec = 0.05f;
    float mod = 3.0f, mfrq = 2.5f, mdec = 0.03f, hpfHz = 1500.0f;

    float baseFreq = 400.0f;
    float velocity = 1.0f;
    bool active = false;
};
