#include "VantageSynthesiser.h"
#include <algorithm>

namespace vantage
{
    void VantageSynthesiser::setMonoMode(bool shouldBeMono)
    {
        if (shouldBeMono != monoMode)
            heldNotes.clear();
        monoMode = shouldBeMono;
    }

    SynthVoice* VantageSynthesiser::getMonoVoice()
    {
        return dynamic_cast<SynthVoice*>(getVoice(0));
    }

    void VantageSynthesiser::noteOn(int midiChannel, int midiNoteNumber, float velocity)
    {
        if (!monoMode)
        {
            Synthesiser::noteOn(midiChannel, midiNoteNumber, velocity);
            return;
        }

        const juce::ScopedLock sl(lock);

        auto* voice = getMonoVoice();
        auto sound = getSound(0);
        if (voice == nullptr || sound == nullptr)
            return;

        const bool wasSounding = !heldNotes.empty();
        heldNotes.push_back({ midiChannel, midiNoteNumber, velocity });

        if (wasSounding && legato)
        {
            voice->glideToNote(midiNoteNumber, velocity);
        }
        else
        {
            voice->setGlideOnNextStart(wasSounding);
            startVoice(voice, sound.get(), midiChannel, midiNoteNumber, velocity);
        }
    }

    void VantageSynthesiser::noteOff(int midiChannel, int midiNoteNumber, float velocity, bool allowTailOff)
    {
        if (!monoMode)
        {
            Synthesiser::noteOff(midiChannel, midiNoteNumber, velocity, allowTailOff);
            return;
        }

        const juce::ScopedLock sl(lock);

        heldNotes.erase(std::remove_if(heldNotes.begin(), heldNotes.end(),
            [midiChannel, midiNoteNumber](const HeldNote& n)
            {
                return n.channel == midiChannel && n.note == midiNoteNumber;
            }), heldNotes.end());

        auto* voice = getMonoVoice();
        if (voice == nullptr)
            return;

        if (heldNotes.empty())
        {
            stopVoice(voice, velocity, allowTailOff);
        }
        else
        {
            const auto& previous = heldNotes.back();
            if (legato)
            {
                voice->glideToNote(previous.note, previous.velocity);
            }
            else if (auto sound = getSound(0))
            {
                voice->setGlideOnNextStart(true);
                startVoice(voice, sound.get(), previous.channel, previous.note, previous.velocity);
            }
        }
    }
}
