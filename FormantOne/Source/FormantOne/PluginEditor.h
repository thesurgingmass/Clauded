#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>
#include "PluginProcessor.h"
#include "FormantOneLookAndFeel.h"

namespace formantone
{
    /** One vertical fader bound to an APVTS float parameter, with a short
        caption below it and a value popup that appears only while
        dragging (matching a dense hardware-style dashboard, which doesn't
        dedicate space to a permanent numeric readout per fader). */
    class FaderControl : public juce::Component
    {
    public:
        FaderControl(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText);
        void resized() override;

    private:
        juce::Slider slider { juce::Slider::LinearVertical, juce::Slider::NoTextBox };
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    /** A caption + combo box bound to an APVTS choice parameter. */
    class ChoiceControl : public juce::Component
    {
    public:
        ChoiceControl(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText);
        void resized() override;

    private:
        juce::ComboBox combo;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    };

    /** A toggle button bound to an APVTS bool parameter. */
    class ToggleControl : public juce::Component
    {
    public:
        ToggleControl(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText);
        void resized() override;

    private:
        juce::ToggleButton button;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attachment;
    };

    /** Lays out its children in one row of equal-width slots — the basic
        building block every panel's control strip is made of. */
    class FaderRow : public juce::Component
    {
    public:
        void addItem(juce::Component* item); // takes ownership
        void resized() override;

    private:
        juce::OwnedArray<juce::Component> items;
    };

    /** A titled, bordered dashboard panel — the visual chrome shared by
        every section (Operator, Filter, LFO, ...). Subclasses fill in
        `getContentBounds()`. */
    class Panel : public juce::Component
    {
    public:
        explicit Panel(juce::String titleText);
        void paint(juce::Graphics&) override;

        static constexpr int headerHeight = 22;

    protected:
        juce::Rectangle<int> getContentBounds() const;

    private:
        juce::String title;
    };

    /** A panel whose whole content is one row of faders/controls. */
    class SimpleFaderPanel : public Panel
    {
    public:
        explicit SimpleFaderPanel(const juce::String& titleText);
        void addItem(juce::Component* item); // takes ownership
        void resized() override;

    private:
        FaderRow row;
    };

    /** A panel with prev/next arrows in its header that page through N
        near-identical items (operators, formants), showing one item's
        FaderRow at a time — mirrors the oscillator-select arrows on
        hardware-style synth GUIs, instead of a full tab per item. */
    class PagedFaderPanel : public Panel
    {
    public:
        PagedFaderPanel(const juce::String& titleText, int numPages, std::function<juce::String(int)> pageLabel);
        FaderRow& getPage(int index) { return *pages[index]; }
        void resized() override;

    private:
        void showPage(int index);

        juce::OwnedArray<FaderRow> pages;
        juce::TextButton prevButton { "<" }, nextButton { ">" };
        juce::Label indexLabel;
        std::function<juce::String(int)> pageLabelFn;
        int currentPage = 0;
    };

    /** A small live-updating preview of the LFO's current waveform shape. */
    class LfoWavePreview : public juce::Component, private juce::Timer
    {
    public:
        explicit LfoWavePreview(juce::AudioProcessorValueTreeState& apvts);
        void paint(juce::Graphics&) override;

    private:
        void timerCallback() override { repaint(); }
        juce::AudioProcessorValueTreeState& apvts;
    };

    /** The LFO panel: shape selector + waveform preview side by side on
        top, rate/delay/depth faders along the bottom. */
    class LfoPanel : public Panel
    {
    public:
        LfoPanel(juce::AudioProcessorValueTreeState& apvts);
        void resized() override;

    private:
        ChoiceControl shapeControl;
        LfoWavePreview preview;
        FaderRow row;
    };

    class FormantOneAudioProcessorEditor : public juce::AudioProcessorEditor
    {
    public:
        explicit FormantOneAudioProcessorEditor(FormantOneAudioProcessor&);
        ~FormantOneAudioProcessorEditor() override;

        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        FormantOneAudioProcessor& processorRef;
        FormantOneLookAndFeel lookAndFeel;

        juce::OwnedArray<juce::Component> topRowPanels;
        juce::Array<float> topRowWeights;
        juce::OwnedArray<juce::Component> bottomRowPanels;
        juce::Array<float> bottomRowWeights;

        void layoutRow(juce::Rectangle<int> bounds, const juce::OwnedArray<juce::Component>& panels, const juce::Array<float>& weights);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FormantOneAudioProcessorEditor)
    };
}
