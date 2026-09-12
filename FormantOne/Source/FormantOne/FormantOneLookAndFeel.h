#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace formantone
{
    /**
        A warm, dark hardware-synth-style theme (dark bronze panels, a
        copper accent, cream text) with a custom vertical-fader drawing —
        JUCE's default linear slider is a thin line with a small round
        thumb, which reads poorly at the fader-strip sizes a dense,
        Surge-XT-style dashboard needs. Buttons/combo boxes are recoloured
        via colour IDs rather than custom-drawn, which is enough to make
        them read as part of the same theme without needing bespoke
        drawing code for every widget type.
    */
    class FormantOneLookAndFeel : public juce::LookAndFeel_V4
    {
    public:
        FormantOneLookAndFeel();

        void drawLinearSlider(juce::Graphics&, int x, int y, int width, int height,
                               float sliderPos, float minSliderPos, float maxSliderPos,
                               juce::Slider::SliderStyle, juce::Slider&) override;

        static const juce::Colour background;
        static const juce::Colour panelFill;
        static const juce::Colour panelBorder;
        static const juce::Colour accent;
        static const juce::Colour text;
    };
}
