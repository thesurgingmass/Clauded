#include "FormantOneLookAndFeel.h"

namespace formantone
{
    const juce::Colour FormantOneLookAndFeel::background  = juce::Colour(0xff241c17);
    const juce::Colour FormantOneLookAndFeel::panelFill    = juce::Colour(0xff3a2e26);
    const juce::Colour FormantOneLookAndFeel::panelBorder  = juce::Colour(0xff8a6a4a);
    const juce::Colour FormantOneLookAndFeel::accent       = juce::Colour(0xffe8a13c);
    const juce::Colour FormantOneLookAndFeel::text         = juce::Colour(0xffe8d9c5);

    FormantOneLookAndFeel::FormantOneLookAndFeel()
    {
        setColourScheme(getDarkColourScheme());

        setColour(juce::ResizableWindow::backgroundColourId, background);
        setColour(juce::Slider::trackColourId, juce::Colour(0xff1c1512));
        setColour(juce::Slider::backgroundColourId, juce::Colour(0xff1c1512));
        setColour(juce::Slider::thumbColourId, accent);
        setColour(juce::Slider::textBoxTextColourId, text);
        setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

        setColour(juce::Label::textColourId, text);

        setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff2a211c));
        setColour(juce::ComboBox::outlineColourId, panelBorder);
        setColour(juce::ComboBox::textColourId, text);
        setColour(juce::ComboBox::arrowColourId, accent);

        setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2a211c));
        setColour(juce::TextButton::buttonOnColourId, accent);
        setColour(juce::TextButton::textColourOffId, text);
        setColour(juce::TextButton::textColourOnId, juce::Colour(0xff241c17));

        setColour(juce::ToggleButton::textColourId, text);
        setColour(juce::ToggleButton::tickColourId, accent);
        setColour(juce::ToggleButton::tickDisabledColourId, panelBorder);

        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff2a211c));
        setColour(juce::PopupMenu::textColourId, text);
        setColour(juce::PopupMenu::highlightedBackgroundColourId, accent);
    }

    void FormantOneLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                                  float sliderPos, float minSliderPos, float maxSliderPos,
                                                  juce::Slider::SliderStyle style, juce::Slider& slider)
    {
        if (style != juce::Slider::LinearVertical)
        {
            LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
            return;
        }

        juce::ignoreUnused(minSliderPos, maxSliderPos);

        auto bounds = juce::Rectangle<float>(static_cast<float>(x), static_cast<float>(y),
                                              static_cast<float>(width), static_cast<float>(height));
        const float trackWidth = juce::jmin(8.0f, bounds.getWidth() * 0.28f);
        auto track = bounds.withSizeKeepingCentre(trackWidth, bounds.getHeight());

        g.setColour(slider.findColour(juce::Slider::backgroundColourId));
        g.fillRoundedRectangle(track, trackWidth * 0.5f);

        const float fillTop = juce::jlimit(track.getY(), track.getBottom(), sliderPos);
        auto fill = track.withTop(fillTop);
        if (fill.getHeight() > 1.0f)
        {
            g.setColour(FormantOneLookAndFeel::accent);
            g.fillRoundedRectangle(fill, trackWidth * 0.5f);
        }

        // Fader cap: a horizontal bar centred on the current value.
        const float capWidth = bounds.getWidth();
        const float capHeight = 10.0f;
        auto cap = juce::Rectangle<float>(bounds.getX(), sliderPos - capHeight * 0.5f, capWidth, capHeight);
        g.setColour(juce::Colour(0xff17110d));
        g.fillRoundedRectangle(cap, 2.5f);
        g.setColour(slider.findColour(juce::Slider::thumbColourId));
        g.drawRoundedRectangle(cap.reduced(0.5f), 2.5f, 1.4f);
        g.fillRect(cap.withSizeKeepingCentre(capWidth * 0.6f, 2.0f));
    }
}
