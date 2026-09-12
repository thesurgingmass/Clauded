#include "PluginEditor.h"
#include <cmath>

namespace formantone
{
    namespace
    {
        void addEnvelopeFaders(juce::AudioProcessorValueTreeState& apvts, SimpleFaderPanel& panel, const juce::String& owner)
        {
            for (const auto& stage : getEnvelopeStages())
                panel.addItem(new FaderControl(apvts, ParamIDs::envelopeId(owner, stage.idSuffix), stage.shortLabel));
        }

        void addEnvelopeFaders(juce::AudioProcessorValueTreeState& apvts, FaderRow& row, const juce::String& owner)
        {
            for (const auto& stage : getEnvelopeStages())
                row.addItem(new FaderControl(apvts, ParamIDs::envelopeId(owner, stage.idSuffix), stage.shortLabel));
        }

        /** A thin vertical rule, purely decorative — separates an item's
            main controls from its envelope faders within the same page so
            the two groups don't visually blur together. */
        class Divider : public juce::Component
        {
            void paint(juce::Graphics& g) override
            {
                g.setColour(FormantOneLookAndFeel::panelBorder.withAlpha(0.5f));
                g.fillRect(getLocalBounds().withSizeKeepingCentre(1, getHeight()));
            }
        };
    }

    // ---- FaderControl -----------------------------------------------------

    FaderControl::FaderControl(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText)
    {
        addAndMakeVisible(slider);
        slider.setPopupDisplayEnabled(true, false, this);
        addAndMakeVisible(label);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::FontOptions(10.5f));
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID, slider);
    }

    void FaderControl::resized()
    {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromBottom(14));
        slider.setBounds(bounds);
    }

    // ---- ChoiceControl ---------------------------------------------------------

    ChoiceControl::ChoiceControl(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText)
    {
        addAndMakeVisible(label);
        label.setText(labelText, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::FontOptions(10.5f));

        addAndMakeVisible(combo);
        if (auto* param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(paramID)))
            combo.addItemList(param->choices, 1);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(apvts, paramID, combo);
    }

    void ChoiceControl::resized()
    {
        auto bounds = getLocalBounds();
        label.setBounds(bounds.removeFromTop(14));
        combo.setBounds(bounds.removeFromTop(22));
    }

    // ---- ToggleControl ----------------------------------------------------------

    ToggleControl::ToggleControl(juce::AudioProcessorValueTreeState& apvts, const juce::String& paramID, const juce::String& labelText)
    {
        addAndMakeVisible(button);
        button.setButtonText(labelText);
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, paramID, button);
    }

    void ToggleControl::resized()
    {
        button.setBounds(getLocalBounds().withSizeKeepingCentre(getWidth(), 22));
    }

    // ---- FaderRow -----------------------------------------------------------

    void FaderRow::addItem(juce::Component* item)
    {
        items.add(item);
        addAndMakeVisible(item);
    }

    void FaderRow::resized()
    {
        auto bounds = getLocalBounds();
        const int n = juce::jmax(1, items.size());
        const int slotWidth = bounds.getWidth() / n;
        for (int i = 0; i < items.size(); ++i)
        {
            auto slot = juce::Rectangle<int>(bounds.getX() + i * slotWidth, bounds.getY(), slotWidth, bounds.getHeight());
            items[i]->setBounds(slot.reduced(3, 0));
        }
    }

    // ---- Panel --------------------------------------------------------------

    Panel::Panel(juce::String titleText) : title(std::move(titleText)) {}

    void Panel::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat();
        g.setColour(FormantOneLookAndFeel::panelFill);
        g.fillRoundedRectangle(bounds, 6.0f);
        g.setColour(FormantOneLookAndFeel::panelBorder);
        g.drawRoundedRectangle(bounds.reduced(0.75f), 6.0f, 1.4f);

        g.setColour(FormantOneLookAndFeel::text);
        g.setFont(juce::FontOptions(13.0f, juce::Font::bold));
        g.drawText(title, juce::Rectangle<int>(10, 3, getWidth() - 56, headerHeight - 2), juce::Justification::centredLeft);
    }

    juce::Rectangle<int> Panel::getContentBounds() const
    {
        return getLocalBounds().withTrimmedTop(headerHeight).reduced(6);
    }

    // ---- SimpleFaderPanel -------------------------------------------------------

    SimpleFaderPanel::SimpleFaderPanel(const juce::String& titleText) : Panel(titleText)
    {
        addAndMakeVisible(row);
    }

    void SimpleFaderPanel::addItem(juce::Component* item)
    {
        row.addItem(item);
    }

    void SimpleFaderPanel::resized()
    {
        row.setBounds(getContentBounds());
    }

    // ---- PagedFaderPanel --------------------------------------------------------

    PagedFaderPanel::PagedFaderPanel(const juce::String& titleText, int numPages, std::function<juce::String(int)> pageLabel)
        : Panel(titleText), pageLabelFn(std::move(pageLabel))
    {
        for (int i = 0; i < numPages; ++i)
        {
            auto* page = new FaderRow();
            pages.add(page);
            addChildComponent(page);
        }

        addAndMakeVisible(prevButton);
        addAndMakeVisible(nextButton);
        addAndMakeVisible(indexLabel);
        indexLabel.setJustificationType(juce::Justification::centred);
        indexLabel.setFont(juce::FontOptions(11.0f));

        prevButton.onClick = [this] { showPage(currentPage - 1); };
        nextButton.onClick = [this] { showPage(currentPage + 1); };

        showPage(0);
    }

    void PagedFaderPanel::showPage(int index)
    {
        index = juce::jlimit(0, pages.size() - 1, index);
        if (!pages.isEmpty())
            pages[currentPage]->setVisible(false);

        currentPage = index;
        pages[currentPage]->setVisible(true);
        indexLabel.setText(pageLabelFn(currentPage), juce::dontSendNotification);
        prevButton.setEnabled(currentPage > 0);
        nextButton.setEnabled(currentPage < pages.size() - 1);
    }

    void PagedFaderPanel::resized()
    {
        auto header = getLocalBounds().removeFromTop(headerHeight);
        nextButton.setBounds(header.removeFromRight(18).reduced(1, 2));
        indexLabel.setBounds(header.removeFromRight(22));
        prevButton.setBounds(header.removeFromRight(18).reduced(1, 2));

        auto content = getContentBounds();
        for (auto* page : pages)
            page->setBounds(content);
    }

    // ---- LfoWavePreview -----------------------------------------------------

    LfoWavePreview::LfoWavePreview(juce::AudioProcessorValueTreeState& apvtsRef) : apvts(apvtsRef)
    {
        startTimerHz(8);
    }

    void LfoWavePreview::paint(juce::Graphics& g)
    {
        auto bounds = getLocalBounds().toFloat().reduced(3.0f);
        g.setColour(juce::Colour(0xff1c1512));
        g.fillRoundedRectangle(bounds, 4.0f);

        const int shape = static_cast<int>(apvts.getRawParameterValue(ParamIDs::lfoShape())->load());
        constexpr int numPoints = 96;

        juce::Path path;
        for (int i = 0; i < numPoints; ++i)
        {
            const float t = static_cast<float>(i) / static_cast<float>(numPoints - 1);
            float v = 0.0f;
            switch (shape)
            {
                case 0: v = std::sin(t * juce::MathConstants<float>::twoPi); break;                     // sine
                case 1: v = t < 0.5f ? (4.0f * t - 1.0f) : (3.0f - 4.0f * t); break;                     // triangle
                case 2: v = 2.0f * t - 1.0f; break;                                                      // saw
                case 3: v = t < 0.5f ? 1.0f : -1.0f; break;                                               // square
                default: v = 2.0f * (std::fmod(std::floor(t * 8.0f) * 0.6180339887f, 1.0f)) - 1.0f; break; // sample & hold
            }
            const float x = bounds.getX() + t * bounds.getWidth();
            const float y = bounds.getCentreY() - v * bounds.getHeight() * 0.38f;
            if (i == 0)
                path.startNewSubPath(x, y);
            else
                path.lineTo(x, y);
        }

        g.setColour(FormantOneLookAndFeel::accent);
        g.strokePath(path, juce::PathStrokeType(1.6f));
    }

    // ---- LfoPanel -------------------------------------------------------------

    LfoPanel::LfoPanel(juce::AudioProcessorValueTreeState& apvts)
        : Panel("LFO"), shapeControl(apvts, ParamIDs::lfoShape(), "Shape"), preview(apvts)
    {
        addAndMakeVisible(shapeControl);
        addAndMakeVisible(preview);
        addAndMakeVisible(row);

        row.addItem(new FaderControl(apvts, ParamIDs::lfoRate(), "Rate"));
        row.addItem(new FaderControl(apvts, ParamIDs::lfoDelay(), "Delay"));
        row.addItem(new FaderControl(apvts, ParamIDs::lfoPitchDepth(), "Pitch"));
        row.addItem(new FaderControl(apvts, ParamIDs::lfoAmpDepth(), "Amp"));
        row.addItem(new FaderControl(apvts, ParamIDs::lfoFilterDepth(), "Filter"));
    }

    void LfoPanel::resized()
    {
        auto bounds = getContentBounds();
        auto top = bounds.removeFromTop(bounds.getHeight() * 5 / 9);
        shapeControl.setBounds(top.removeFromLeft(90));
        preview.setBounds(top.reduced(4, 0));
        row.setBounds(bounds);
    }

    // ---- FormantOneAudioProcessorEditor ------------------------------------------

    void FormantOneAudioProcessorEditor::layoutRow(juce::Rectangle<int> bounds, const juce::OwnedArray<juce::Component>& panels,
                                                    const juce::Array<float>& weights)
    {
        float totalWeight = 0.0f;
        for (auto w : weights)
            totalWeight += w;

        int xPos = bounds.getX();
        const int gap = 6;
        const int usableWidth = bounds.getWidth() - gap * (panels.size() - 1);

        for (int i = 0; i < panels.size(); ++i)
        {
            const int w = static_cast<int>(usableWidth * (weights[i] / totalWeight));
            panels[i]->setBounds(xPos, bounds.getY(), w, bounds.getHeight());
            xPos += w + gap;
        }
    }

    FormantOneAudioProcessorEditor::FormantOneAudioProcessorEditor(FormantOneAudioProcessor& p)
        : juce::AudioProcessorEditor(&p), processorRef(p)
    {
        setLookAndFeel(&lookAndFeel);

        auto& apvts = processorRef.apvts;

        // ---- Top row ----

        auto* operatorPanel = new PagedFaderPanel("Operator", numOperators, [](int i) { return juce::String(i + 1); });
        for (int i = 0; i < numOperators; ++i)
        {
            auto& opRow = operatorPanel->getPage(i);
            opRow.addItem(new ChoiceControl(apvts, ParamIDs::opRatioCoarse(i), "Ratio"));
            opRow.addItem(new FaderControl(apvts, ParamIDs::opRatioFine(i), "Fine"));
            opRow.addItem(new ToggleControl(apvts, ParamIDs::opFixedMode(i), "Fixed"));
            opRow.addItem(new FaderControl(apvts, ParamIDs::opFixedFreq(i), "Fixed Hz"));
            opRow.addItem(new FaderControl(apvts, ParamIDs::opLevel(i), "Level"));
            if (i == 0)
                opRow.addItem(new FaderControl(apvts, ParamIDs::opFeedback(), "FdBk"));
            opRow.addItem(new Divider());
            addEnvelopeFaders(apvts, opRow, ParamIDs::opOwner(i));
        }
        topRowPanels.add(operatorPanel);
        topRowWeights.add(4.4f);

        auto* algorithmPanel = new SimpleFaderPanel("Algorithm");
        algorithmPanel->addItem(new ChoiceControl(apvts, ParamIDs::algorithm(), "Algorithm"));
        topRowPanels.add(algorithmPanel);
        topRowWeights.add(1.1f);

        auto* filterPanel = new SimpleFaderPanel("Filter");
        filterPanel->addItem(new ChoiceControl(apvts, ParamIDs::filterType(), "Type"));
        filterPanel->addItem(new FaderControl(apvts, ParamIDs::filterCutoff(), "Cutoff"));
        filterPanel->addItem(new FaderControl(apvts, ParamIDs::filterResonance(), "Reso"));
        filterPanel->addItem(new FaderControl(apvts, ParamIDs::filterEnvAmount(), "EnvAmt"));
        filterPanel->addItem(new FaderControl(apvts, ParamIDs::filterKeyTrack(), "KeyTrk"));
        topRowPanels.add(filterPanel);
        topRowWeights.add(2.0f);

        auto* filterEnvPanel = new SimpleFaderPanel("Filter Envelope");
        addEnvelopeFaders(apvts, *filterEnvPanel, ParamIDs::filterEnvOwner());
        topRowPanels.add(filterEnvPanel);
        topRowWeights.add(2.2f);

        auto* outputPanel = new SimpleFaderPanel("Balance / Output");
        outputPanel->addItem(new FaderControl(apvts, ParamIDs::voicedUnvoiced(), "Voice"));
        outputPanel->addItem(new FaderControl(apvts, ParamIDs::fmFormantBalance(), "FM<>Fmt"));
        outputPanel->addItem(new FaderControl(apvts, ParamIDs::driveAmount(), "Drive"));
        outputPanel->addItem(new FaderControl(apvts, ParamIDs::outputGain(), "Gain"));
        topRowPanels.add(outputPanel);
        topRowWeights.add(1.8f);

        // ---- Bottom row ----

        auto* formantPanel = new PagedFaderPanel("Formant", 3, [](int i) { return juce::String(i + 1); });
        for (int f = 0; f < 3; ++f)
        {
            auto& fRow = formantPanel->getPage(f);
            fRow.addItem(new FaderControl(apvts, ParamIDs::formantFrequency(f), "Freq"));
            fRow.addItem(new FaderControl(apvts, ParamIDs::formantBandwidth(f), "BW"));
            fRow.addItem(new FaderControl(apvts, ParamIDs::formantLevel(f), "Level"));
            fRow.addItem(new Divider());
            addEnvelopeFaders(apvts, fRow, ParamIDs::formantOwner(f));
        }
        bottomRowPanels.add(formantPanel);
        bottomRowWeights.add(3.6f);

        auto* lfoPanel = new LfoPanel(apvts);
        bottomRowPanels.add(lfoPanel);
        bottomRowWeights.add(2.6f);

        auto* ampEnvPanel = new SimpleFaderPanel("Amp Envelope");
        addEnvelopeFaders(apvts, *ampEnvPanel, ParamIDs::ampOwner());
        bottomRowPanels.add(ampEnvPanel);
        bottomRowWeights.add(2.2f);

        for (auto* c : topRowPanels)
            addAndMakeVisible(c);
        for (auto* c : bottomRowPanels)
            addAndMakeVisible(c);

        setResizable(true, true);
        setResizeLimits(1000, 620, 1900, 1150);
        setSize(1450, 820);
    }

    FormantOneAudioProcessorEditor::~FormantOneAudioProcessorEditor()
    {
        setLookAndFeel(nullptr);
    }

    void FormantOneAudioProcessorEditor::paint(juce::Graphics& g)
    {
        g.fillAll(FormantOneLookAndFeel::background);
    }

    void FormantOneAudioProcessorEditor::resized()
    {
        auto bounds = getLocalBounds().reduced(8);
        const int rowGap = 8;
        const int rowHeight = (bounds.getHeight() - rowGap) / 2;

        layoutRow(bounds.removeFromTop(rowHeight), topRowPanels, topRowWeights);
        bounds.removeFromTop(rowGap);
        layoutRow(bounds, bottomRowPanels, bottomRowWeights);
    }
}
