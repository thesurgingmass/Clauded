#include "PluginEditor.h"

namespace fmtone
{
    // ---- ParamSlider ------------------------------------------------------

    ParamSlider::ParamSlider(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText)
    {
        addAndMakeVisible(slider);
        addAndMakeVisible(label);
        label.setText(labelText.toUpperCase(), juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(fm::FMLookAndFeel::monoFont(11.0f, true));
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 84, 16);
        slider.setNumDecimalPlacesToDisplay(2);
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
        label.setText(labelText.toUpperCase(), juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(fm::FMLookAndFeel::monoFont(11.0f, true));

        if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(paramID)))
            combo.addItemList(param->choices, 1);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, paramID, combo);
    }

    void ParamCombo::resized()
    {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromTop(16));
        combo.setBounds(bounds.reduced(0, 10));
    }

    // ---- ControlSection -----------------------------------------------------

    void ControlSection::addControl(juce::Component* control)
    {
        controls.add(control);
        addAndMakeVisible(control);
    }

    void ControlSection::resized()
    {
        auto bounds = getLocalBounds().reduced(16);

        juce::FlexBox flex;
        flex.flexDirection = juce::FlexBox::Direction::row;
        flex.flexWrap = juce::FlexBox::Wrap::wrap;
        flex.alignContent = juce::FlexBox::AlignContent::flexStart;
        flex.alignItems = juce::FlexBox::AlignItems::flexStart;

        for (auto* control : controls)
            flex.items.add(juce::FlexItem(*control).withMinWidth(96.0f).withMinHeight(104.0f).withMargin(8.0f));

        flex.performLayout(bounds);
    }

    // ---- FMToneAudioProcessorEditor ------------------------------------------

    FMToneAudioProcessorEditor::FMToneAudioProcessorEditor(FMToneAudioProcessor& p)
        : juce::AudioProcessorEditor(&p), processorRef(p)
    {
        setLookAndFeel(&lookAndFeel);

        auto& apvts = processorRef.apvts;

        auto& algoSection = addSection("ALG");
        algoSection.addControl(new ParamCombo(apvts, ParamIDs::algorithm(), "Algorithm"));

        for (int i = 0; i < numOperators; ++i)
        {
            auto& opSection = addSection(juce::String("OP") + operatorNames[i]);
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

        auto& ampSection = addSection("AMP");
        ampSection.addControl(new ParamSlider(apvts, ParamIDs::ampAttack(), "Attack"));
        ampSection.addControl(new ParamSlider(apvts, ParamIDs::ampDecay(), "Decay"));
        ampSection.addControl(new ParamSlider(apvts, ParamIDs::ampSustain(), "Sustain"));
        ampSection.addControl(new ParamSlider(apvts, ParamIDs::ampRelease(), "Release"));

        auto& filterSection = addSection("FLTR");
        filterSection.addControl(new ParamCombo(apvts, ParamIDs::filterType(), "Type"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterCutoff(), "Cutoff"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterResonance(), "Reso"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterEnvAmount(), "Env Amt"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterKeyTrack(), "Key Track"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterAttack(), "Attack"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterDecay(), "Decay"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterSustain(), "Sustain"));
        filterSection.addControl(new ParamSlider(apvts, ParamIDs::filterRelease(), "Release"));

        auto& outputSection = addSection("OUT");
        outputSection.addControl(new ParamSlider(apvts, ParamIDs::driveAmount(), "Drive"));
        outputSection.addControl(new ParamSlider(apvts, ParamIDs::outputGain(), "Gain"));

        setResizable(true, true);
        setResizeLimits(700, 400, 1400, 1600);
        setSize(900, 460);
    }

    FMToneAudioProcessorEditor::~FMToneAudioProcessorEditor()
    {
        setLookAndFeel(nullptr);
    }

    ControlSection& FMToneAudioProcessorEditor::addSection(const juce::String& tagLabel)
    {
        auto* section = new ControlSection(tagLabel);
        const int index = sections.size();
        sections.add(section);
        addAndMakeVisible(section);
        section->setVisible(index == 0);

        auto* tab = new juce::TextButton(tagLabel);
        tab->setClickingTogglesState(true);
        tab->setRadioGroupId(1, juce::dontSendNotification);
        tab->setToggleState(index == 0, juce::dontSendNotification);
        tab->onClick = [this, index] { showSection(index); };
        tabButtons.add(tab);
        addAndMakeVisible(tab);

        return *section;
    }

    void FMToneAudioProcessorEditor::showSection(int index)
    {
        for (int i = 0; i < sections.size(); ++i)
            sections[i]->setVisible(i == index);
    }

    void FMToneAudioProcessorEditor::paint(juce::Graphics& g)
    {
        g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

        // Faint scanlines across the whole window for a bit of screen texture.
        g.setColour(fm::FMLookAndFeel::ink().withAlpha(0.025f));
        for (int lineY = 0; lineY < getHeight(); lineY += 3)
            g.drawLine(0.0f, (float) lineY, (float) getWidth(), (float) lineY, 1.0f);

        auto header = headerBounds.toFloat().reduced(8.0f, 6.0f);
        g.setColour(fm::FMLookAndFeel::ink());
        g.drawRect(header, 1.5f);

        auto inner = header.reduced(10.0f, 6.0f);

        auto tagBox = inner.removeFromLeft(46.0f);
        g.drawRect(tagBox, 1.5f);
        g.setFont(fm::FMLookAndFeel::monoFont(13.0f, true));
        g.drawText("01", tagBox, juce::Justification::centred);

        inner.removeFromLeft(12.0f);

        auto voxBox = inner.removeFromRight(90.0f);
        g.drawRect(voxBox, 1.5f);
        g.setFont(fm::FMLookAndFeel::monoFont(13.0f, true));
        g.drawText(juce::String(numVoices) + " VOX", voxBox, juce::Justification::centred);

        inner.removeFromRight(12.0f);

        g.setFont(fm::FMLookAndFeel::monoFont(24.0f, true));
        g.drawText("FM TONE", inner, juce::Justification::centredLeft);
    }

    void FMToneAudioProcessorEditor::resized()
    {
        auto area = getLocalBounds();
        headerBounds = area.removeFromTop(headerHeight);

        auto tabBar = area.removeFromTop(tabBarHeight).reduced(8, 4);
        juce::FlexBox tabFlex;
        tabFlex.flexDirection = juce::FlexBox::Direction::row;
        for (auto* tab : tabButtons)
            tabFlex.items.add(juce::FlexItem(*tab).withFlex(1.0f).withMargin(2.0f));
        tabFlex.performLayout(tabBar);

        for (auto* section : sections)
            section->setBounds(area);
    }
}
