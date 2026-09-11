#include "RateLevelEnvelope.h"
#include <algorithm>

namespace fs
{
    void RateLevelEnvelope::reset()
    {
        stage = Stage::idle;
        currentLevel = 0.0f;
        segmentSamplesRemaining = 0;
    }

    void RateLevelEnvelope::enterStage(Stage newStage)
    {
        stage = newStage;
        segmentStartLevel = currentLevel;

        float timeSeconds = 0.0f;
        switch (stage)
        {
            case Stage::attack:  segmentTargetLevel = params.level1; timeSeconds = params.attackSeconds; break;
            case Stage::decay1:  segmentTargetLevel = params.level2; timeSeconds = params.decay1Seconds; break;
            case Stage::decay2:  segmentTargetLevel = params.level3; timeSeconds = params.decay2Seconds; break;
            case Stage::release: segmentTargetLevel = params.level4; timeSeconds = params.releaseSeconds; break;
            case Stage::sustainHold:
            case Stage::idle:
            default:
                segmentTargetLevel = currentLevel;
                segmentSamplesRemaining = 0;
                segmentIncrement = 0.0f;
                return;
        }

        segmentSamplesRemaining = std::max(1, static_cast<int>(timeSeconds * sampleRate));
        segmentIncrement = (segmentTargetLevel - segmentStartLevel) / static_cast<float>(segmentSamplesRemaining);
    }

    void RateLevelEnvelope::noteOn()
    {
        currentLevel = 0.0f;
        enterStage(Stage::attack);
    }

    void RateLevelEnvelope::noteOff()
    {
        // Interrupts whatever segment is currently running (including a
        // sustain hold) and releases from the level the envelope is
        // actually at right now, not from level3.
        if (stage != Stage::idle)
            enterStage(Stage::release);
    }

    float RateLevelEnvelope::getNextSample()
    {
        if (stage == Stage::idle)
            return 0.0f;

        if (stage == Stage::sustainHold)
            return currentLevel;

        currentLevel += segmentIncrement;
        --segmentSamplesRemaining;

        if (segmentSamplesRemaining <= 0)
        {
            currentLevel = segmentTargetLevel;

            switch (stage)
            {
                case Stage::attack:  enterStage(Stage::decay1); break;
                case Stage::decay1:  enterStage(Stage::decay2); break;
                case Stage::decay2:  enterStage(Stage::sustainHold); break;
                case Stage::release: stage = Stage::idle; break;
                default: break;
            }
        }

        return currentLevel;
    }
}
