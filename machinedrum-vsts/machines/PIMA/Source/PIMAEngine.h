#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"
#include "md/dsp/Filter.h"

/** PI-MA: physically-informed maracas/shaker. SYN params: GRNS, DEC, GLEN,
    SIZE, HARD. A simplified stand-in for a granular "particle" shaker
    model (the classic technique for physically modelling maracas is many
    small particles colliding inside a shell): a randomly gated,
    bandpass-filtered noise texture whose gate rate is set by GRNS (grain
    density) and average segment length by GLEN, with HARD sharpening each
    grain's attack and SIZE setting the shell's resonant center frequency
    (lower = bigger shell). DEC shapes the overall one-shot shake event. */
class PIMAEngine : public md::MachineEngine
{
public:
    void prepare(double sampleRate) override;
    void reset() override;
    void trigger(float velocity01, float pitchOffsetSemitones) override;
    void setSynParams(const std::vector<float>& v) override;
    float renderSample() override;
    bool isActive() const override { return active; }

private:
    md::NoiseGen noise, gateNoise;
    md::AHDEnvelope ampEnv;
    md::StateVariableFilter bandpass;

    double sampleRate = 44100.0;
    float grns = 0.5f, dec = 0.3f, glen = 0.5f, size = 0.5f, hard = 0.4f;

    float gateLevel = 0.0f, gateTarget = 0.0f;
    int gateCounter = 0;

    float velocity = 1.0f;
    bool active = false;
};
