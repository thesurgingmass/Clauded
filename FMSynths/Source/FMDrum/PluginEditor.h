#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "Common/FMLookAndFeel.h"

namespace fmdrum
{
    /** A label + rotary knob icon bound to an APVTS parameter, LCD-style. */
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

    /** One full editor "page" of controls - only one of these is visible at a time. */
    class ControlSection : public juce::Component
    {
    public:
        explicit ControlSection(const juce::String& tagLabel) : tag(tagLabel) {}
        void addControl(juce::Component* control); // takes ownership
        void resized() override;
        const juce::String& getTag() const { return tag; }

    private:
        juce::String tag;
        juce::OwnedArray<juce::Component> controls;
    };

    class FMDrumAudioProcessorEditor : public juce::AudioProcessorEditor
    {
    public:
        explicit FMDrumAudioProcessorEditor(FMDrumAudioProcessor&);
        ~FMDrumAudioProcessorEditor() override;

        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        FMDrumAudioProcessor& processorRef;
        fm::FMLookAndFeel lookAndFeel;

        juce::OwnedArray<ControlSection> sections;
        juce::OwnedArray<juce::TextButton> tabButtons;

        juce::Rectangle<int> headerBounds;
        static constexpr int headerHeight = 56;
        static constexpr int tabBarHeight = 40;

        ControlSection& addSection(const juce::String& tagLabel);
        void showSection(int index);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FMDrumAudioProcessorEditor)
    };
}
