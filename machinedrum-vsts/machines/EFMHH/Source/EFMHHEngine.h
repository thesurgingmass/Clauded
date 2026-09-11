#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/FMVoice.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"

/** EFM-HH: FM hi-hat. SYN params: PTCH, DEC, TREM, TFRQ, MOD, MFRQ, MDEC,
    FB. An FM carrier/modulator pair with self-feedback (FB) generates the
    inharmonic metallic tone; TREM/TFRQ apply the machine's own internal
    tremolo (distinct from the shared TFX page's AMD/AMF) for the shimmering
    hi-hat texture. */
class EFMHHEngine : public md::MachineEngine
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

    double sampleRate = 44100.0;
    float tremPhase = 0.0f;

    float ptchHz = 3000.0f, dec = 0.2f, trem = 0.3f, tfrq = 40.0f;
    float mod = 4.0f, mfrq = 3.7f, mdec = 0.06f, fb = 0.3f;

    float baseFreq = 3000.0f;
    float velocity = 1.0f;
    bool active = false;
};
