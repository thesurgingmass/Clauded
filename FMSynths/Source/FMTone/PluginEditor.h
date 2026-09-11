#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "Common/FMLookAndFeel.h"

namespace fmtone
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

    /** A row of controls under a small boxed section tag, LCD-page style. */
    class ControlSection : public juce::Component
    {
    public:
        explicit ControlSection(const juce::String& tagLabel);
        void addControl(juce::Component* control); // takes ownership
        void resized() override;
        void paint(juce::Graphics&) override;
        int getPreferredHeight() const { return preferredHeight; }

    private:
        juce::String tag;
        juce::OwnedArray<juce::Component> controls;
        int preferredHeight = 118;
        static constexpr int chipHeight = 22;
    };

    class FMToneAudioProcessorEditor : public juce::AudioProcessorEditor
    {
    public:
        explicit FMToneAudioProcessorEditor(FMToneAudioProcessor&);
        ~FMToneAudioProcessorEditor() override;

        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        FMToneAudioProcessor& processorRef;
        fm::FMLookAndFeel lookAndFeel;

        juce::Viewport viewport;
        juce::Component content;
        juce::OwnedArray<ControlSection> sections;

        juce::Rectangle<int> headerBounds;
        static constexpr int headerHeight = 56;

        ControlSection& addSection(const juce::String& tagLabel);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FMToneAudioProcessorEditor)
    };
}
