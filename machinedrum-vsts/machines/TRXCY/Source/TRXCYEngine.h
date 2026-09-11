#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"
#include "md/dsp/Filter.h"
#include <array>

/** TRX-CY: classic analog cymbal. SYN params: RICH, DEC, TOP, TTUN, SIZE,
    PEAK. A cluster of inharmonic square oscillators forms the wash (SIZE
    lowers the base frequency for a "bigger" cymbal, RICH fades in more
    partials for a denser/richer spectrum); an internal resonant bandpass
    (TTUN center, PEAK resonance) is blended in at TOP amount for the bright
    top layer. */
class TRXCYEngine : public md::MachineEngine
{
public:
    void prepare(double sampleRate) override;
    void reset() override;
    void trigger(float velocity01, float pitchOffsetSemitones) override;
    void setSynParams(const std::vector<float>& v) override;
    float renderSample() override;
    bool isActive() const override { return active; }

private:
    std::array<md::AnalogOsc, 6> oscs;
    md::AHDEnvelope ampEnv;
    md::StateVariableFilter topFilter;

    double sampleRate = 44100.0;
    float rich = 0.6f, dec = 1.2f, top = 0.4f, ttun = 8000.0f, size = 0.4f, peak = 0.2f;

    float velocity = 1.0f;
    bool active = false;
};
