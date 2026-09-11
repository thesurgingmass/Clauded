#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/FMVoice.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"

/** EFM-XT: FM tom. SYN params: PTCH, DEC, RAMP, RDEC, MOD, MFRQ, NDEC, CLIC.
    Like EFM-BD, adds a pitch-drop sweep (RAMP/RDEC) on top of the FM tone,
    but the modulation index shares the body's DEC (no separate MDEC on this
    machine), and adds an independent noise layer (decaying over NDEC) and
    click transient (CLIC) instead of self-feedback. */
class EFMXTEngine : public md::MachineEngine
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
    md::PitchDropEnvelope rampEnv;
    md::AHDEnvelope bodyEnv, noiseEnv, clickEnv;

    double sampleRate = 44100.0;
    float ptchHz = 120.0f, dec = 0.3f, ramp = -12.0f, rdec = 0.06f;
    float mod = 2.5f, mfrq = 1.8f, ndec = 0.1f, clic = 0.2f;

    float baseFreq = 120.0f;
    float velocity = 1.0f;
    bool active = false;
};
