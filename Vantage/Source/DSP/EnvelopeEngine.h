#pragma once

namespace vantage
{
    /**
     * SH-101-style envelope: exponential attack/decay/release segments
     * (each an analog-style asymptotic curve toward its target, rather than
     * a linear ramp) with a fast, decisive attack and a natural decaying
     * curve on release — the snappy, punchy character the hardware is
     * known for, as opposed to a flat linear ADSR.
     */
    class EnvelopeEngine
    {
    public:
        void prepare(double newSampleRate);
        void reset();

        void setParameters(float attackSeconds, float decaySeconds, float sustain01, float releaseSeconds);

        void noteOn();
        void noteOff();
        bool isActive() const { return stage != Stage::Idle; }

        float renderSample();

    private:
        enum class Stage { Idle, Attack, Decay, Sustain, Release };

        static float coefficientFor(float timeSeconds, double sampleRate);

        double sampleRate = 44100.0;
        Stage stage = Stage::Idle;
        float value = 0.0f;

        float attackCoeff = 0.0f;
        float decayCoeff = 0.0f;
        float releaseCoeff = 0.0f;
        float sustainLevel = 0.7f;
    };
}
