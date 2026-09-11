#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/FMVoice.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"
#include "md/dsp/Filter.h"

/** EFM-CY: FM cymbal. SYN params: PTCH, DEC, FB, HPF, MOD, MFRQ, MDEC. An
    FM carrier/modulator pair with self-feedback (FB) generates the
    inharmonic wash, shaped by an internal highpass (HPF) for brightness and
    a longer decay (DEC) than the hi-hat machine. */
class EFMCYEngine : public md::MachineEngine
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
    md::AHDEnvelope bodyEnv, modEnv;
    md::StateVariableFilter hpf;

    double sampleRate = 44100.0;
    float ptchHz = 2200.0f, dec = 1.2f, fb = 0.35f, hpfHz = 2500.0f;
    float mod = 4.5f, mfrq = 2.3f, mdec = 0.3f;

    float baseFreq = 2200.0f;
    float velocity = 1.0f;
    bool active = false;
};
