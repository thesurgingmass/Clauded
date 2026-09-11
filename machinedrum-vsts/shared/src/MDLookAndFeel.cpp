#include "md/MDLookAndFeel.h"

namespace md
{
    const juce::Colour MDLookAndFeel::chassisDark   { 0xff1c1c1c };
    const juce::Colour MDLookAndFeel::chassisPanel  { 0xff2a2a2a };
    const juce::Colour MDLookAndFeel::elektronOrange{ 0xffe8792c };
    const juce::Colour MDLookAndFeel::knobBody      { 0xff3a3a3a };
    const juce::Colour MDLookAndFeel::knobRing      { 0xff141414 };
    const juce::Colour MDLookAndFeel::textCream     { 0xffe8dfc8 };

    MDLookAndFeel::MDLookAndFeel()
    {
        setColour(juce::ResizableWindow::backgroundColourId, chassisDark);
        setColour(juce::Slider::textBoxTextColourId, textCream);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
        setColour(juce::Label::textColourId, textCream);
        setColour(juce::ToggleButton::textColourId, textCream);
    }

    void MDLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                          juce::Slider&)
    {
        auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
        auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
        auto centre = bounds.getCentre();
        auto angle  = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // Outer track (unfilled arc)
        juce::Path track;
        track.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
        g.setColour(knobRing);
        g.strokePath(track, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Filled value arc
        juce::Path valueArc;
        valueArc.addCentredArc(centre.x, centre.y, radius, radius, 0.0f, rotaryStartAngle, angle, true);
        g.setColour(elektronOrange);
        g.strokePath(valueArc, juce::PathStrokeType(3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // Knob body
        auto knobRadius = radius * 0.72f;
        auto knobBounds = juce::Rectangle<float>(knobRadius * 2.0f, knobRadius * 2.0f).withCentre(centre);
        juce::ColourGradient grad(knobBody.brighter(0.25f), centre.x, centre.y - knobRadius,
                                   knobBody.darker(0.4f), centre.x, centre.y + knobRadius, false);
        g.setGradientFill(grad);
        g.fillEllipse(knobBounds);
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.drawEllipse(knobBounds, 1.0f);

        // Pointer
        juce::Path pointer;
        auto pointerLength = knobRadius * 0.78f;
        auto pointerThickness = 2.4f;
        pointer.addRectangle(-pointerThickness * 0.5f, -pointerLength, pointerThickness, pointerLength * 0.55f);
        pointer.applyTransform(juce::AffineTransform::rotation(angle).translated(centre));
        g.setColour(elektronOrange);
        g.fillPath(pointer);
    }

    void MDLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                          bool shouldDrawButtonAsHighlighted, bool)
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
        auto ledSize = juce::jmin(14.0f, bounds.getHeight() * 0.5f);
        auto ledBounds = juce::Rectangle<float>(ledSize, ledSize).withCentre({ bounds.getX() + ledSize, bounds.getCentreY() });

        g.setColour(chassisPanel.darker(0.2f));
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(shouldDrawButtonAsHighlighted ? juce::Colours::grey : juce::Colours::black.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);

        g.setColour(button.getToggleState() ? elektronOrange : juce::Colour(0xff4a2a18));
        g.fillEllipse(ledBounds);

        g.setColour(textCream);
        g.setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
        g.drawText(button.getButtonText(), bounds.withTrimmedLeft(ledSize + 8.0f), juce::Justification::centredLeft);
    }

    void MDLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
    {
        g.setColour(label.findColour(juce::Label::textColourId));
        g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
        g.drawText(label.getText(), label.getLocalBounds(), label.getJustificationType());
    }
}
