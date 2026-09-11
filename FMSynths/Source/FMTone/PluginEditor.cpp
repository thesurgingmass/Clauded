#include "PluginEditor.h"

namespace fmtone
{
    // ---- ParamSlider ------------------------------------------------------

    ParamSlider::ParamSlider(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText)
    {
        addAndMakeVisible(slider);
        addAndMakeVisible(label);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::FontOptions(12.0f));
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID, slider);
    }

    void ParamSlider::resized()
    {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromTop(16));
        slider.setBounds(bounds);
    }

    // ---- ParamCombo ---------------------------------------------------------

    ParamCombo::ParamCombo(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText)
    {
        addAndMakeVisible(combo);
        addAndMakeVisible(label);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::FontOptions(12.0f));

        if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(paramID)))
            combo.addItemList(param->choices, 1);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, paramID, combo);
    }

    void ParamCombo::resized()
    {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromTop(16));
        combo.setBounds(bounds.removeFromTop(24));
    }

    // ---- ControlSection -----------------------------------------------------

    ControlSection::ControlSection(const juce::String& title) : group(title, title)
    {
        addAndMakeVisible(group);
        group.setTextLabelPosition(juce::Justification::centredLeft);
    }

    void ControlSection::addControl(juce::Component* control)
    {
        controls.add(control);
        addAndMakeVisible(control);
    }

    void ControlSection::resized()
    {
        group.setBounds(getLocalBounds());

        auto bounds = getLocalBounds().reduced(10, 24);
        juce::FlexBox flex;
        flex.flexDirection = juce::FlexBox::Direction::row;
        flex.flexWrap = juce::FlexBox::Wrap::wrap;
        flex.alignContent = juce::FlexBox::AlignContent::flexStart;

        for (auto* control : controls)
            flex.items.add(juce::FlexItem(*control).withMinWidth(80.0f).withMinHeight(80.0f).withMargin(4.0f));

        flex.performLayout(bounds);
    }

    // ---- FMToneAudioProcessorEditor ------------------------------------------

    FMToneAudioProcessorEditor::FMToneAudioProcessorEditor(FMToneAudioProcessor& p)
        : juce::AudioProcessorEditor(&p), processorRef(p)
    {
        auto& apvts = processorRef.apvts;

        auto& algoSection = addSection("Algorithm");
        algoSection.addControl(new ParamCombo(apvts, ParamIDs::algorithm(), "Algorithm"));

        for (int i = 0; i < numOperators; ++i)
        {
            auto& opSection = addSection(juce::String("Operator ") + operatorNames[i]);
            opSection.addControl(new ParamCombo(apvts, ParamIDs::opRatioCoarse(i), "Ratio"));
            opSection.addControl(new ParamSlider(apvts, ParamIDs::opRatioFine(i), "Fine"));
            opSection.addControl(new ParamSlider(apvts, ParamIDs::opLevel(i), "Level"));
            if (i == opC)
                opSection.addControl(new ParamSlider(apvts, ParamIDs::opFeedback(), "Feedback"));
            opSection.addControl(new ParamSlider(apvts, ParamIDs::opAttack(i), "Attack"));
            opSection.addControl(new ParamSlider(apvts, ParamIDs::opDecay(i), "Decay"));
            opSection.addControl(new ParamSlider(apvts, ParamIDs::opSustain(i), "End Lvl"));
            opSection.addControl(new ParamSlider(apvts, ParamIDs::opRelease(i), "Release"));
        }

        auto& ampSection = addSection("Amp Envelope");
        ampSection.addControl(new ParamSlider(apvts, ParamIDs::ampAttack(), "Attack"));
        ampSection.addControl(new ParamSlider(apvts, ParamIDs::ampDecay(), "Decay"));
        ampSection.addControl(new ParamSlider(apvts, ParamIDs::ampSustain(), "Sustain"));
        ampSection.addControl(new ParamSlider(apvts, ParamIDs::ampRelease(), "Release"));

        auto& filterSection = addSection("Filter");
        filterSection.addControl(new ParamCombo(apvts, ParamIDs::filterType(), "Type"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterCutoff(), "Cutoff"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterResonance(), "Reso"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterEnvAmount(), "Env Amt"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterKeyTrack(), "Key Track"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterAttack(), "Attack"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterDecay(), "Decay"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterSustain(), "Sustain"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterRelease(), "Release"));

        auto& outputSection = addSection("Output");
        outputSection.addControl(new ParamSlider(apvts, ParamIDs::driveAmount(), "Drive"));
        outputSection.addControl(new ParamSlider(apvts, ParamIDs::outputGain(), "Gain"));

        addAndMakeVisible(viewport);
        viewport.setViewedComponent(&content, false);

        for (auto* section : sections)
            content.addAndMakeVisible(section);

        setResizable(true, true);
        setResizeLimits(700, 400, 1400, 1600);
        setSize(900, 720);
    }

    ControlSection& FMToneAudioProcessorEditor::addSection(const juce::String& title)
    {
        auto* section = new ControlSection(title);
        sections.add(section);
        return *section;
    }

    void FMToneAudioProcessorEditor::paint(juce::Graphics& g)
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }

    void FMToneAudioProcessorEditor::resized()
    {
        viewport.setBounds(getLocalBounds());

        const int contentWidth = juce::jmax(getWidth() - viewport.getScrollBarThickness(), 600);
        int y = 0;
        for (auto* section : sections)
        {
            section->setBounds(0, y, contentWidth, section->getPreferredHeight());
            y += section->getPreferredHeight();
        }
        content.setSize(contentWidth, y);
    }
}
