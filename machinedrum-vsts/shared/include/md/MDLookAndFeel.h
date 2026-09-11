#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

namespace md
{
    /** LookAndFeel styled after the Elektron Machinedrum SPS-1's faceplate:
        dark charcoal chassis, an amber/orange LCD-style display, and
        machined-metal knobs with an orange indicator line. */
    class MDLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        MDLookAndFeel();

        void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                               float sliderPosProportional, float rotaryStartAngle,
                               float rotaryEndAngle, juce::Slider&) override;

        void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

        void drawLabel(juce::Graphics&, juce::Label&) override;

        static const juce::Colour chassisDark;
        static const juce::Colour chassisPanel;
        static const juce::Colour elektronOrange;
        static const juce::Colour knobBody;
        static const juce::Colour knobRing;
        static const juce::Colour textCream;
    };
}
