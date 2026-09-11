#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace fmdrum
{
    /** A label + rotary slider bound to an APVTS parameter, laid out as one unit. */
    class ParamSlider : public juce::Component
    {
    public:
        ParamSlider(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText);
        void resized() override;

    private:
        juce::Slider slider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    /** A label + combo box bound to an APVTS choice parameter. */
    class ParamCombo : public juce::Component
    {
    public:
        ParamCombo(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText);
        void resized() override;

    private:
        juce::ComboBox combo;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    };

    /** A titled group of controls laid out left-to-right. */
    class ControlSection : public juce::Component
    {
    public:
        explicit ControlSection(const juce::String& title);
        void addControl(juce::Component* control); // takes ownership
        void resized() override;
        int getPreferredHeight() const { return preferredHeight; }

    private:
        juce::GroupComponent group;
        juce::OwnedArray<juce::Component> controls;
        int preferredHeight = 110;
    };

    class FMDrumAudioProcessorEditor : public juce::AudioProcessorEditor
    {
    public:
        explicit FMDrumAudioProcessorEditor(FMDrumAudioProcessor&);
        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        FMDrumAudioProcessor& processorRef;

        juce::Viewport viewport;
        juce::Component content;
        juce::OwnedArray<ControlSection> sections;

        ControlSection& addSection(const juce::String& title);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FMDrumAudioProcessorEditor)
    };
}
