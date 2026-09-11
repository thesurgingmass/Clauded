#include "PluginEditor.h"

namespace fmdrum
{
    // ---- ParamSlider ------------------------------------------------------

    ParamSlider::ParamSlider(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText)
    {
        addAndMakeVisible(slider);
        addAndMakeVisible(label);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font(12.0f));
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 18);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID, slider);
    }

    void ParamSlider::resized()
    {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromTop(16));
        slider.setBounds(bounds);
    }

    // ---- ParamCombo -----------------------------------------------------

    ParamCombo::ParamCombo(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText)
    {
        addAndMakeVisible(combo);
        addAndMakeVisible(label);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::Font(12.0f));

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

    // ---- ControlSection ---------------------------------------------------

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

    // ---- FMDrumAudioProcessorEditor ----------------------------------------

    FMDrumAudioProcessorEditor::FMDrumAudioProcessorEditor(FMDrumAudioProcessor& p)
        : juce::AudioProcessorEditor(&p), processorRef(p)
    {
        auto& apvts = processorRef.apvts;

        auto& bodySection = addSection("Body (FM)");
        bodySection.addControl(new ParamSlider(apvts, ParamIDs::bodyRatio(), "Ratio"));
        bodySection.addControl(new ParamSlider(apvts, ParamIDs::bodyTune(), "Tune"));
        bodySection.addControl(new ParamSlider(apvts, ParamIDs::bodyIndex(), "Index"));
        bodySection.addControl(new ParamSlider(apvts, ParamIDs::bodyFeedback(), "Feedback"));
        bodySection.addControl(new ParamSlider(apvts, ParamIDs::bodyLevel(), "Level"));

        auto& pitchSection = addSection("Pitch Sweep");
        pitchSection.addControl(new ParamSlider(apvts, ParamIDs::pitchEnvAmount(), "Amount"));
        pitchSection.addControl(new ParamSlider(apvts, ParamIDs::pitchEnvDecay(), "Decay"));

        auto& noiseSection = addSection("Noise");
        noiseSection.addControl(new ParamSlider(apvts, ParamIDs::noiseLevel(), "Level"));
        noiseSection.addControl(new ParamCombo(apvts, ParamIDs::noiseFilterType(), "Filter"));
        noiseSection.addControl(new ParamSlider(apvts, ParamIDs::noiseFilterCutoff(), "Tone"));
        noiseSection.addControl(new ParamSlider(apvts, ParamIDs::noiseAttack(), "Attack"));
        noiseSection.addControl(new ParamSlider(apvts, ParamIDs::noiseDecay(), "Decay"));
        noiseSection.addControl(new ParamSlider(apvts, ParamIDs::noiseSustain(), "Sustain"));
        noiseSection.addControl(new ParamSlider(apvts, ParamIDs::noiseRelease(), "Release"));

        auto& transientSection = addSection("Transient");
        transientSection.addControl(new ParamSlider(apvts, ParamIDs::transientLevel(), "Level"));
        transientSection.addControl(new ParamSlider(apvts, ParamIDs::transientDecay(), "Decay"));

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

    ControlSection& FMDrumAudioProcessorEditor::addSection(const juce::String& title)
    {
        auto* section = new ControlSection(title);
        sections.add(section);
        return *section;
    }

    void FMDrumAudioProcessorEditor::paint(juce::Graphics& g)
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }

    void FMDrumAudioProcessorEditor::resized()
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
