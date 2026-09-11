#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace formantone
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

    /** A label + toggle button bound to an APVTS bool parameter. */
    class ParamToggle : public juce::Component
    {
    public:
        ParamToggle(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText);
        void resized() override;

    private:
        juce::ToggleButton button;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
    };

    /** A titled group of controls laid out left-to-right, wrapping onto as
        many rows as the given width requires. */
    class ControlSection : public juce::Component
    {
    public:
        explicit ControlSection(const juce::String& title);
        void addControl(juce::Component* control); // takes ownership
        void resized() override;

        /** How tall this section needs to be to fit all its controls
            (without clipping) at a given width — sections vary a lot in
            control count here (an operator section has a dozen; the
            balance section has two), so height can't be a fixed constant
            the way a small, uniform control set could get away with. */
        int computePreferredHeight(int width) const;

    private:
        juce::GroupComponent group;
        juce::OwnedArray<juce::Component> controls;

        static constexpr int itemSize = 80;
        static constexpr int itemMargin = 4;
        static constexpr int horizontalPadding = 20;  // matches reduced(10, ...)
        static constexpr int verticalPadding = 48;    // matches reduced(..., 24) top+bottom
    };

    class FormantOneAudioProcessorEditor : public juce::AudioProcessorEditor
    {
    public:
        explicit FormantOneAudioProcessorEditor(FormantOneAudioProcessor&);
        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        FormantOneAudioProcessor& processorRef;

        juce::Viewport viewport;
        juce::Component content;
        juce::OwnedArray<ControlSection> sections;

        ControlSection& addSection(const juce::String& title);
        void addEnvelopeControls(ControlSection& section, const juce::String& owner);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormantOneAudioProcessorEditor)
    };
}
