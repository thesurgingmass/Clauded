#pragma once
#include "md/MachineEngine.h"
#include "md/dsp/Oscillators.h"
#include "md/dsp/Envelope.h"
#include "md/dsp/Filter.h"
#include "md/dsp/Shaper.h"

/** TRX-CB: classic analog cowbell. SYN params: PTCH, DEC, ENH, DAMP, TONE,
    BUMP. Two square oscillators at the classic ~1:1.48 cowbell ratio form
    the metallic body; ENH wavefolds in extra harmonics; an internal
    lowpass (DAMP amount, TONE center) shapes brightness; BUMP layers a low
    sine thump under the attack. */
class TRXCBEngine : public md::MachineEngine
{
public:
    void prepare(double sampleRate) override;
    void reset() override;
    void trigger(float velocity01, float pitchOffsetSemitones) override;
    void setSynParams(const std::vector<float>& v) override;
    float renderSample() override;
    bool isActive() const override { return active; }

private:
    md::AnalogOsc osc1, osc2, bumpOsc;
    md::AHDEnvelope ampEnv, bumpEnv;
    md::StateVariableFilter lp;

    double sampleRate = 44100.0;
    float ptchHz = 540.0f, dec = 0.3f, enh = 0.3f, damp = 0.3f, tone = 4000.0f, bump = 0.2f;

    float baseFreq = 540.0f;
    float velocity = 1.0f;
    bool active = false;
};
