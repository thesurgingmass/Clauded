#include "md/MachineEditor.h"
#include "md/MachineProcessor.h"

namespace md
{
    MachineEditor::MachineEditor(MachineProcessor& p)
        : juce::AudioProcessorEditor(&p), proc(p)
    {
        setLookAndFeel(&lookAndFeel);

        std::vector<std::pair<juce::String, juce::String>> synIds;
        for (auto& sp : proc.getMachineInfo().synParams)
            synIds.push_back({ sp.id, sp.name });
        buildRow(synRow, "SYN", synIds);

        buildRow(tfxRow, "TFX", {
            { "AMD", "AMD" }, { "AMF", "AMF" }, { "EQF", "EQF" }, { "EQG", "EQG" },
            { "FLTF", "FLTF" }, { "FLTW", "FLTW" }, { "FLTQ", "FLTQ" }, { "SRR", "SRR" }
        });

        buildRow(routRow, "ROUT", {
            { "DIST", "DIST" }, { "VOL", "VOL" }, { "PAN", "PAN" }, { "DEL", "DEL" }, { "REV", "REV" }
        });

        levSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 56, 16);
        addAndMakeVisible(levSlider);
        levLabel.setText("LEV", juce::dontSendNotification);
        levLabel.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(levLabel);
        levAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            proc.apvts, "LEV", levSlider);

        kybdButton.setClickingTogglesState(true);
        addAndMakeVisible(kybdButton);
        kybdAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
            proc.apvts, "KYBD", kybdButton);

        setSize(820, 480);
    }

    MachineEditor::~MachineEditor()
    {
        setLookAndFeel(nullptr);
    }

    void MachineEditor::buildRow(KnobRow& row, const juce::String& title,
                                  const std::vector<std::pair<juce::String, juce::String>>& idsAndNames)
    {
        row.title = title;
        for (auto& [id, name] : idsAndNames)
        {
            auto slider = std::make_unique<juce::Slider>(juce::Slider::RotaryHorizontalVerticalDrag,
                                                           juce::Slider::TextBoxBelow);
            slider->setTextBoxStyle(juce::Slider::TextBoxBelow, true, 56, 16);
            addAndMakeVisible(*slider);

            auto label = std::make_unique<juce::Label>();
            label->setText(name, juce::dontSendNotification);
            label->setJustificationType(juce::Justification::centred);
            addAndMakeVisible(*label);

            row.attachments.push_back(std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
                proc.apvts, id, *slider));

            row.sliders.push_back(std::move(slider));
            row.labels.push_back(std::move(label));
        }
    }

    void MachineEditor::paint(juce::Graphics& g)
    {
        g.fillAll(MDLookAndFeel::chassisDark);

        auto bounds = getLocalBounds();
        auto header = bounds.removeFromTop(64);
        auto levArea = header.removeFromRight(80);
        juce::ignoreUnused(levArea);

        // LCD-style display
        auto lcd = header.reduced(12, 8);
        g.setColour(juce::Colour(0xff120a00));
        g.fillRoundedRectangle(lcd.toFloat(), 4.0f);
        g.setColour(MDLookAndFeel::elektronOrange.withAlpha(0.85f));
        g.drawRoundedRectangle(lcd.toFloat(), 4.0f, 1.0f);

        g.setColour(MDLookAndFeel::elektronOrange);
        g.setFont(juce::Font(juce::FontOptions(13.0f)).withExtraKerningFactor(0.15f));
        auto textArea = lcd.reduced(12, 0);
        g.drawText(proc.getMachineInfo().category, textArea.removeFromLeft(80),
                   juce::Justification::centredLeft);
        g.setFont(juce::Font(juce::FontOptions(22.0f, juce::Font::bold)).withExtraKerningFactor(0.1f));
        g.drawText(proc.getMachineInfo().machineName, textArea, juce::Justification::centredLeft);

        bounds.removeFromBottom(50);
        auto rowHeight = bounds.getHeight() / 3;
        paintRow(g, synRow, bounds.removeFromTop(rowHeight));
        paintRow(g, tfxRow, bounds.removeFromTop(rowHeight));
        paintRow(g, routRow, bounds.removeFromTop(rowHeight));

        g.setColour(MDLookAndFeel::chassisPanel);
        g.fillRect(getLocalBounds().removeFromBottom(50));
    }

    void MachineEditor::paintRow(juce::Graphics& g, const KnobRow& row, juce::Rectangle<int> area)
    {
        g.setColour(MDLookAndFeel::chassisPanel);
        g.fillRect(area);
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.drawLine((float) area.getX(), (float) area.getBottom(), (float) area.getRight(), (float) area.getBottom());

        auto titleArea = area.removeFromLeft(46);
        g.setColour(MDLookAndFeel::elektronOrange);
        g.fillRect(titleArea.removeFromLeft(4));
        g.setColour(MDLookAndFeel::textCream);
        g.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(-juce::MathConstants<float>::halfPi,
                                                         (float) titleArea.getCentreX(), (float) titleArea.getCentreY()));
        g.drawText(row.title, titleArea.withSizeKeepingCentre(area.getHeight(), 40), juce::Justification::centred);
        g.restoreState();
    }

    void MachineEditor::resized()
    {
        auto bounds = getLocalBounds();
        auto header = bounds.removeFromTop(64);
        auto levArea = header.removeFromRight(80).reduced(4);
        levLabel.setBounds(levArea.removeFromTop(14));
        levSlider.setBounds(levArea);

        auto bottom = bounds.removeFromBottom(50);
        kybdButton.setBounds(bottom.reduced(12, 10).removeFromLeft(160));

        auto rowHeight = bounds.getHeight() / 3;
        layoutRow(synRow, bounds.removeFromTop(rowHeight));
        layoutRow(tfxRow, bounds.removeFromTop(rowHeight));
        layoutRow(routRow, bounds.removeFromTop(rowHeight));
    }

    void MachineEditor::layoutRow(KnobRow& row, juce::Rectangle<int> area)
    {
        area.removeFromLeft(46);
        area.reduce(10, 6);
        if (row.sliders.empty())
            return;

        auto slotWidth = area.getWidth() / (int) row.sliders.size();
        for (size_t i = 0; i < row.sliders.size(); ++i)
        {
            auto slot = area.removeFromLeft(slotWidth);
            auto labelArea = slot.removeFromTop(16);
            row.labels[i]->setBounds(labelArea);
            row.sliders[i]->setBounds(slot.reduced(6, 0));
        }
    }
}
