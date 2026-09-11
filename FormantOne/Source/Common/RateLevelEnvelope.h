#pragma once

namespace fs
{
    /**
        A 4-stage rate/level envelope, the same shape as the operator EG on
        classic Yamaha FM hardware (DX/TX/FS-series): three time+level pairs
        that run while a note is held, plus a fourth that runs on release.

        The behaviorally important, well-documented part of these hardware
        envelopes (not just their four-segment shape) is what happens at
        note-off: the envelope doesn't wait to reach L3 before it can
        release. It holds at L3 once it gets there, but if note-off arrives
        *before* it gets there, the envelope jumps straight from whatever
        level it's currently at into the R4/L4 release segment. That
        interrupt-from-anywhere behavior is what this class models; the
        segments themselves are simple linear ramps in level over time
        (a reasonable, transparent approximation — Yamaha never published
        the exact per-rate timing curve its hardware used for the 0-99 rate
        dial, so this isn't an attempt to reproduce that curve bit-for-bit).
    */
    class RateLevelEnvelope
    {
    public:
        struct Parameters
        {
            float attackSeconds = 0.001f;   // R1: idle level (0) -> level1
            float level1 = 1.0f;
            float decay1Seconds = 0.25f;    // R2: level1 -> level2
            float level2 = 0.8f;
            float decay2Seconds = 0.25f;    // R3: level2 -> level3 (sustain)
            float level3 = 0.8f;
            float releaseSeconds = 0.2f;    // R4: current level -> level4, starts at note-off
            float level4 = 0.0f;
        };

        void setSampleRate(double newSampleRate) { sampleRate = newSampleRate; }
        void setParameters(const Parameters& newParams) { params = newParams; }

        void reset();
        void noteOn();
        void noteOff();

        bool isActive() const { return stage != Stage::idle; }

        float getNextSample();

    private:
        enum class Stage { idle, attack, decay1, decay2, sustainHold, release };

        void enterStage(Stage newStage);

        Parameters params;
        double sampleRate = 44100.0;

        Stage stage = Stage::idle;
        float currentLevel = 0.0f;
        float segmentStartLevel = 0.0f;
        float segmentTargetLevel = 0.0f;
        float segmentIncrement = 0.0f;
        int segmentSamplesRemaining = 0;
    };
}
