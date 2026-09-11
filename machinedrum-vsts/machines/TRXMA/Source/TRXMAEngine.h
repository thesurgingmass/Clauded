#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"
#include "md/dsp/Filter.h"
#include "md/dsp/Shaper.h"

/** TRX-MA: analog maracas. SYN params: ATT, SUS, REV, DAMP, RATL, RTYP,
    TONE, HARD. Bandpass-filtered noise (TONE center) shaped by an
    attack/hold/release envelope (ATT/SUS/REV map directly onto the shared
    AHD envelope shape) with an internal lowpass damping stage (DAMP);
    RATL/RTYP add the characteristic "shaking seeds" amplitude-modulated
    rattle, blending between a smooth tremolo and a random sample-and-hold
    modulation; HARD adds transient sharpness at the attack. */
class TRXMAEngine : public md::MachineEngine
{
public:
    void prepare(double sampleRate) override;
    void reset() override;
    void trigger(float velocity01, float pitchOffsetSemitones) override;
    void setSynParams(const std::vector<float>& v) override;
    float renderSample() override;
    bool isActive() const override { return active; }

private:
    md::NoiseGen noise;
    md::AHDEnvelope ampEnv, hardEnv;
    md::StateVariableFilter bandpass, damp;

    double sampleRate = 44100.0;
    float tremPhase = 0.0f;
    float randHoldValue = 0.0f;
    int randHoldCounter = 0;

    float att = 0.005f, sus = 0.05f, rev = 0.2f, dampAmt = 0.3f;
    float ratl = 0.4f, rtyp = 0.5f, tone = 6000.0f, hard = 0.2f;

    float velocity = 1.0f;
    bool active = false;
};
