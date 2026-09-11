#pragma once
#include <cmath>
#include <algorithm>

namespace md
{
    /** Simple attack/hold/decay envelope with exponential decay curve,
        matching the shape used across the Machinedrum's AMP and FLT pages. */
    class AHDEnvelope
    {
    public:
        void prepare(double sr) { sampleRate = sr; reset(); }

        void reset()
        {
            stage = Stage::Idle;
            value = 0.0f;
            samplesInStage = 0;
        }

        /** attackSec/holdSec/decaySec are in seconds; decayShape 0=linear..1=very exponential. */
        void setTimes(float attackSec, float holdSec, float decaySec)
        {
            attackSamples = std::max(1, (int) (attackSec * sampleRate));
            holdSamples   = (int) (holdSec * sampleRate);
            decaySamples  = std::max(1, (int) (decaySec * sampleRate));
        }

        void trigger()
        {
            stage = Stage::Attack;
            samplesInStage = 0;
            startLevel = value;
        }

        float process()
        {
            switch (stage)
            {
                case Stage::Idle:
                    value = 0.0f;
                    break;

                case Stage::Attack:
                {
                    float t = (float) samplesInStage / (float) attackSamples;
                    value = startLevel + (1.0f - startLevel) * t;
                    if (++samplesInStage >= attackSamples)
                    {
                        value = 1.0f;
                        stage = holdSamples > 0 ? Stage::Hold : Stage::Decay;
                        samplesInStage = 0;
                    }
                    break;
                }

                case Stage::Hold:
                    value = 1.0f;
                    if (++samplesInStage >= holdSamples)
                    {
                        stage = Stage::Decay;
                        samplesInStage = 0;
                    }
                    break;

                case Stage::Decay:
                {
                    // Exponential-ish decay: value *= pow(epsilon, 1/decaySamples)
                    float t = (float) samplesInStage / (float) decaySamples;
                    value = std::pow(1.0f - std::min(t, 1.0f), 1.6f);
                    if (++samplesInStage >= decaySamples)
                    {
                        value = 0.0f;
                        stage = Stage::Idle;
                    }
                    break;
                }
            }
            return value;
        }

        bool isActive() const { return stage != Stage::Idle; }

    private:
        enum class Stage { Idle, Attack, Hold, Decay };
        Stage stage = Stage::Idle;
        double sampleRate = 44100.0;
        int attackSamples = 1, holdSamples = 0, decaySamples = 1;
        int samplesInStage = 0;
        float value = 0.0f;
        float startLevel = 0.0f;
    };

    /** Fast pitch-drop envelope used by kick/tom style machines: starts at a
        high multiple of the base pitch and decays exponentially down to it. */
    class PitchDropEnvelope
    {
    public:
        void prepare(double sr) { sampleRate = sr; }

        void setAmountAndTime(float semitoneAmount, float timeSec)
        {
            amount = semitoneAmount;
            decaySamples = std::max(1, (int) (timeSec * sampleRate));
        }

        void trigger() { samplesInStage = 0; active = true; }

        float process()
        {
            if (! active)
                return 0.0f;

            float t = (float) samplesInStage / (float) decaySamples;
            if (t >= 1.0f) { active = false; return 0.0f; }
            ++samplesInStage;
            return amount * std::pow(1.0f - t, 3.0f);
        }

    private:
        double sampleRate = 44100.0;
        float amount = 0.0f;
        int decaySamples = 1, samplesInStage = 0;
        bool active = false;
    };
}
