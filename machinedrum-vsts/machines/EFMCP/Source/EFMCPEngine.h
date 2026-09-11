#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/FMVoice.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"
#include "md/dsp/Filter.h"
#include <array>

/** EFM-CP: FM-layered handclap. SYN params: PTCH, DEC, CLPS, CDEC, MOD,
    MFRQ, MDEC, HPF. A cluster of short noise bursts (spaced by CLPS) gives
    the characteristic multi-clap flutter, followed by a longer noise tail
    (CDEC); an FM tone (PTCH/DEC/MOD/MFRQ/MDEC) adds body underneath. HPF is
    an internal highpass shaping the noise component specifically (distinct
    from the shared TFX filter). */
class EFMCPEngine : public md::MachineEngine
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
    md::AHDEnvelope bodyEnv, modEnv, tailEnv;
    md::StateVariableFilter hpf;

    double sampleRate = 44100.0;
    float ptchHz = 900.0f, dec = 0.1f, clps = 12.0f, cdec = 0.25f;
    float mod = 1.0f, mfrq = 3.0f, mdec = 0.05f, hpfHz = 800.0f;

    std::array<int, 4> burstStart {};
    int burstLenSamples = 1;
    int samplesSinceTrigger = 0;
    float velocity = 1.0f;
    bool active = false;
};
