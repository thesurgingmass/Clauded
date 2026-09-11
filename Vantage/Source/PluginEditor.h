#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace vantage
{
    /**
     * Placeholder editor: JUCE's built-in generic parameter list, so every
     * APVTS parameter is visible and automatable while the plugin graph is
     * being built out. Replaced by the custom, Surge-XT-inspired UI in a
     * later phase.
     */
    class VantageAudioProcessorEditor : public juce::AudioProcessorEditor
    {
    public:
        explicit VantageAudioProcessorEditor(VantageAudioProcessor& processorToEdit);
        ~VantageAudioProcessorEditor() override = default;

        void resized() override;

    private:
        VantageAudioProcessor& processorRef;
        juce::GenericAudioProcessorEditor genericEditor { processorRef };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VantageAudioProcessorEditor)
    };
}
