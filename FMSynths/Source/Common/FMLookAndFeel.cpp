#include "FMLookAndFeel.h"

namespace fm
{
    FMLookAndFeel::FMLookAndFeel()
    {
        setColour(juce::ResizableWindow::backgroundColourId, background());
        setColour(juce::Label::textColourId, ink());

        setColour(juce::ComboBox::backgroundColourId, background());
        setColour(juce::ComboBox::textColourId, ink());
        setColour(juce::ComboBox::outlineColourId, ink().withAlpha(0.6f));
        setColour(juce::ComboBox::arrowColourId, ink());

        setColour(juce::PopupMenu::backgroundColourId, background());
        setColour(juce::PopupMenu::textColourId, ink());
        setColour(juce::PopupMenu::highlightedBackgroundColourId, ink());
        setColour(juce::PopupMenu::highlightedTextColourId, background());

        setColour(juce::Slider::textBoxTextColourId, ink());
        setColour(juce::Slider::textBoxBackgroundColourId, background());
        setColour(juce::Slider::textBoxOutlineColourId, ink().withAlpha(0.35f));
        setColour(juce::Slider::rotarySliderFillColourId, ink());
        setColour(juce::Slider::rotarySliderOutlineColourId, ink().withAlpha(0.35f));

        setColour(juce::TextEditor::backgroundColourId, background());
        setColour(juce::TextEditor::textColourId, ink());
        setColour(juce::TextEditor::outlineColourId, ink().withAlpha(0.35f));
        setColour(juce::TextEditor::focusedOutlineColourId, ink());

        setColour(juce::ScrollBar::thumbColourId, ink().withAlpha(0.5f));
    }

    juce::Font FMLookAndFeel::monoFont(float size, bool bold)
    {
        auto options = juce::FontOptions {}.withName(juce::Font::getDefaultMonospacedFontName()).withHeight(size);
        if (bold)
            options = options.withStyle("Bold");
        return juce::Font(options);
    }

    juce::Font FMLookAndFeel::getLabelFont(juce::Label&) { return monoFont(13.0f, true); }
    juce::Font FMLookAndFeel::getComboBoxFont(juce::ComboBox&) { return monoFont(13.0f, true); }
    juce::Font FMLookAndFeel::getPopupMenuFont() { return monoFont(13.0f, true); }

    void FMLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                          juce::Slider&)
    {
        // A flat line-art icon - outline circle + a single tick pointer, no
        // fill or gradient - matching how an LCD screen would render a knob:
        // it's just lit or unlit pixels, one colour.
        auto bounds = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height).reduced(4.0f);
        const float diameter = juce::jmin(bounds.getWidth(), bounds.getHeight());
        const float radius = diameter * 0.5f;
        const juce::Point<float> centre = bounds.getCentre();
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        const float ringThickness = juce::jmax(1.5f, radius * 0.09f);
        g.setColour(ink());
        g.drawEllipse(centre.x - radius, centre.y - radius, diameter, diameter, ringThickness);

        juce::Path pointer;
        const float pointerThickness = juce::jmax(2.0f, radius * 0.18f);
        const float pointerInnerGap = radius * 0.15f;
        pointer.addRoundedRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, radius - pointerInnerGap, pointerThickness * 0.4f);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centre.x, centre.y));
        g.setColour(ink());
        g.fillPath(pointer);
    }

    void FMLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                                      int buttonX, int buttonY, int buttonW, int buttonH,
                                      juce::ComboBox&)
    {
        juce::Rectangle<float> box(0.0f, 0.0f, (float) width, (float) height);
        g.setColour(ink().withAlpha(0.7f));
        g.drawRect(box, 1.5f);

        juce::Rectangle<float> arrowZone((float) buttonX, (float) buttonY, (float) buttonW, (float) buttonH);
        juce::Path arrow;
        arrow.startNewSubPath(arrowZone.getX() + arrowZone.getWidth() * 0.22f, arrowZone.getCentreY() - 2.5f);
        arrow.lineTo(arrowZone.getCentreX(), arrowZone.getCentreY() + 3.0f);
        arrow.lineTo(arrowZone.getX() + arrowZone.getWidth() * 0.78f, arrowZone.getCentreY() - 2.5f);

        g.setColour(ink());
        g.strokePath(arrow, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }
}
