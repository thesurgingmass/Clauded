#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>
#include "SynthVoice.h"

namespace vantage
{
    /**
     * Adds mono/legato/glide behavior on top of juce::Synthesiser's normal
     * polyphony. In Poly mode, noteOn/noteOff are untouched (the base
     * class's usual voice-picking). In Mono mode, voice 0 is driven
     * directly from a held-note stack: releasing the most recent note
     * reverts playback to whichever note is still held underneath it
     * (last-note-priority, the classic analog-mono-synth behavior), and
     * Legato decides whether that reverts with a fresh envelope trigger
     * or a seamless pitch glide.
     *
     * Retriggers go through the inherited, protected startVoice()/stopVoice()
     * helpers (not voice->startNote()/stopNote() directly) so the base
     * class's own bookkeeping (currentlyPlayingNote/Sound, keyIsDown) stays
     * correct even in mono mode — e.g. so a note-off still matches up
     * correctly if the host later drops back to Poly mode mid-note.
     */
    class VantageSynthesiser : public juce::Synthesiser
    {
    public:
        void setMonoMode(bool shouldBeMono);
        void setLegato(bool shouldBeLegato) { legato = shouldBeLegato; }

        void noteOn(int midiChannel, int midiNoteNumber, float velocity) override;
        void noteOff(int midiChannel, int midiNoteNumber, float velocity, bool allowTailOff) override;

    private:
        SynthVoice* getMonoVoice();

        struct HeldNote
        {
            int channel;
            int note;
            float velocity;
        };

        bool monoMode = false;
        bool legato = false;
        std::vector<HeldNote> heldNotes;
    };
}
