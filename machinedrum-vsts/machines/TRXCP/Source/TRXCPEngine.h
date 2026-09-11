#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"
#include "md/dsp/Filter.h"
#include "md/dsp/Shaper.h"
#include <array>
#include <vector>

/** TRX-CP: classic analog handclap with a built-in room-style tail. SYN
    params: CLPY, TONE, HARD, RICH, RATE, ROOM, RSIZ, RTUN. A cluster of
    short noise bursts (CLPY spacing) shaped by an internal bandpass (TONE)
    and clipped for sharpness (HARD), optionally thickened by a second
    detuned burst layer (RICH), feeds a small internal comb/allpass "room"
    network (ROOM mix, RSIZ delay size, RTUN feedback damping, RATE subtle
    delay-time modulation) standing in for the clap's characteristic
    ambience -- distinct from, and in addition to, the shared ROUTING REV
    send. */
class TRXCPEngine : public md::MachineEngine
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
    md::AHDEnvelope tailEnv;
    md::StateVariableFilter toneFilter;

    static constexpr int numCombs = 3;
    std::array<std::vector<float>, numCombs> combBuf;
    std::array<size_t, numCombs> combWrite {};
    std::array<float, numCombs> combLast {};
    float roomPhase = 0.0f;

    double sampleRate = 44100.0;
    float clpy = 12.0f, tone = 1200.0f, hard = 0.3f, rich = 0.3f;
    float rate = 2.0f, room = 0.3f, rsiz = 0.4f, rtun = 3000.0f;

    std::array<int, 4> burstStart {};
    int burstLenSamples = 1;
    int samplesSinceTrigger = 0;
    float velocity = 1.0f;
    bool active = false;
};
