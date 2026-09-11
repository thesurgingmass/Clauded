#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>

namespace fm
{
    /**
        Shared visual identity for both plugins: a monochrome lime-on-navy
        look modelled on an actual Elektron hardware LCD screen - flat
        line-art knob icons (outline + tick, no filled arc or gradient),
        blocky monospace caps text, bordered header banner - rather than
        JUCE's stock LookAndFeel_V4 grey.
    */
    class FMLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        FMLookAndFeel();

        void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                               float sliderPosProportional, float rotaryStartAngle,
                               float rotaryEndAngle, juce::Slider&) override;

        void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown,
                           int buttonX, int buttonY, int buttonW, int buttonH,
                           juce::ComboBox&) override;

        juce::Font getLabelFont(juce::Label&) override;
        juce::Font getComboBoxFont(juce::ComboBox&) override;
        juce::Font getPopupMenuFont() override;

        static juce::Colour background() { return juce::Colour(0xff0a0e12); }
        static juce::Colour ink()        { return juce::Colour(0xffc3d732); } // the one LCD colour

        static juce::Font monoFont(float size, bool bold = false);
    };
}
