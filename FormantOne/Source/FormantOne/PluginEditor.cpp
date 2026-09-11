#include "PluginEditor.h"

namespace formantone
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

    // ---- ParamToggle ----------------------------------------------------------

    ParamToggle::ParamToggle(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText)
    {
        addAndMakeVisible(button);
        button.setButtonText(labelText);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, paramID, button);
    }

    void ParamToggle::resized()
    {
        button.setBounds(getLocalBounds());
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

    int ControlSection::computePreferredHeight(int width) const
    {
        const int itemStep = itemSize + itemMargin * 2;
        const int availableWidth = juce::jmax(itemStep, width - horizontalPadding);
        const int itemsPerRow = juce::jmax(1, availableWidth / itemStep);
        const int numControls = juce::jmax(1, controls.size());
        const int numRows = (numControls + itemsPerRow - 1) / itemsPerRow;
        return numRows * itemStep + verticalPadding;
    }

    // ---- FormantOneAudioProcessorEditor ------------------------------------------

    FormantOneAudioProcessorEditor::FormantOneAudioProcessorEditor(FormantOneAudioProcessor& p)
        : juce::AudioProcessorEditor(&p), processorRef(p)
    {
        auto& apvts = processorRef.apvts;

        auto& algoSection = addSection("Algorithm");
        algoSection.addControl(new ParamCombo(apvts, ParamIDs::algorithm(), "Algorithm"));

        for (int i = 0; i < numOperators; ++i)
        {
            const juce::String opName = "Op " + juce::String(i + 1);
            auto& opSection = addSection(opName);
            opSection.addControl(new ParamCombo(apvts, ParamIDs::opRatioCoarse(i), "Ratio"));
            opSection.addControl(new ParamSlider(apvts, ParamIDs::opRatioFine(i), "Fine"));
            opSection.addControl(new ParamToggle(apvts, ParamIDs::opFixedMode(i), "Fixed Freq"));
            opSection.addControl(new ParamSlider(apvts, ParamIDs::opFixedFreq(i), "Fixed Hz"));
            opSection.addControl(new ParamSlider(apvts, ParamIDs::opLevel(i), "Level"));
            if (i == 0)
                opSection.addControl(new ParamSlider(apvts, ParamIDs::opFeedback(), "Feedback"));
            addEnvelopeControls(opSection, ParamIDs::opOwner(i));
        }

        for (int f = 0; f < 3; ++f)
        {
            const juce::String formantName = "Formant " + juce::String(f + 1);
            auto& formantSection = addSection(formantName);
            formantSection.addControl(new ParamSlider(apvts, ParamIDs::formantFrequency(f), "Freq"));
            formantSection.addControl(new ParamSlider(apvts, ParamIDs::formantBandwidth(f), "Bandwidth"));
            formantSection.addControl(new ParamSlider(apvts, ParamIDs::formantLevel(f), "Level"));
            addEnvelopeControls(formantSection, ParamIDs::formantOwner(f));
        }

        auto& balanceSection = addSection("Voiced/Unvoiced & Balance");
        balanceSection.addControl(new ParamSlider(apvts, ParamIDs::voicedUnvoiced(), "Voiced<>Unvoiced"));
        balanceSection.addControl(new ParamSlider(apvts, ParamIDs::fmFormantBalance(), "FM<>Formant"));

        auto& ampSection = addSection("Amp Envelope");
        addEnvelopeControls(ampSection, ParamIDs::ampOwner());

        auto& filterSection = addSection("Filter");
        filterSection.addControl(new ParamCombo(apvts, ParamIDs::filterType(), "Type"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterCutoff(), "Cutoff"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterResonance(), "Reso"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterEnvAmount(), "Env Amt"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterKeyTrack(), "Key Track"));
        addEnvelopeControls(filterSection, ParamIDs::filterEnvOwner());

        auto& lfoSection = addSection("LFO");
        lfoSection.addControl(new ParamCombo(apvts, ParamIDs::lfoShape(), "Shape"));
        lfoSection.addControl(new ParamSlider(apvts, ParamIDs::lfoRate(), "Rate"));
        lfoSection.addControl(new ParamSlider(apvts, ParamIDs::lfoDelay(), "Delay"));
        lfoSection.addControl(new ParamSlider(apvts, ParamIDs::lfoPitchDepth(), "Pitch Depth"));
        lfoSection.addControl(new ParamSlider(apvts, ParamIDs::lfoAmpDepth(), "Amp Depth"));
        lfoSection.addControl(new ParamSlider(apvts, ParamIDs::lfoFilterDepth(), "Filter Depth"));

        auto& outputSection = addSection("Output");
        outputSection.addControl(new ParamSlider(apvts, ParamIDs::driveAmount(), "Drive"));
        outputSection.addControl(new ParamSlider(apvts, ParamIDs::outputGain(), "Gain"));

        addAndMakeVisible(viewport);
        viewport.setViewedComponent(&content, false);

        for (auto* section : sections)
            content.addAndMakeVisible(section);

        setResizable(true, true);
        setResizeLimits(700, 400, 1400, 1600);
        setSize(900, 800);
    }

    ControlSection& FormantOneAudioProcessorEditor::addSection(const juce::String& title)
    {
        auto* section = new ControlSection(title);
        sections.add(section);
        return *section;
    }

    void FormantOneAudioProcessorEditor::addEnvelopeControls(ControlSection& section, const juce::String& owner)
    {
        for (const auto& stage : getEnvelopeStages())
            section.addControl(new ParamSlider(processorRef.apvts, ParamIDs::envelopeId(owner, stage.idSuffix), stage.label));
    }

    void FormantOneAudioProcessorEditor::paint(juce::Graphics& g)
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    }

    void FormantOneAudioProcessorEditor::resized()
    {
        viewport.setBounds(getLocalBounds());

        const int contentWidth = juce::jmax(getWidth() - viewport.getScrollBarThickness(), 600);
        int y = 0;
        for (auto* section : sections)
        {
            const int height = section->computePreferredHeight(contentWidth);
            section->setBounds(0, y, contentWidth, height);
            y += height;
        }
        content.setSize(contentWidth, y);
    }
}
