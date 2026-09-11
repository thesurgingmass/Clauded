#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "MDLookAndFeel.h"

namespace md
{
    class MachineProcessor;

    /** Generic editor for every Machinedrum-inspired machine plugin. Draws a
        faceplate reminiscent of the Machinedrum SPS-1: a dark chassis, an
        amber LCD-style header showing the machine name, and knob rows for
        the SYN (machine-specific), FLT and AMP pages -- LFO intentionally
        omitted. A KYBD MODE toggle switches the voice between one-shot
        drum-trigger behaviour and keyboard pitch-tracking. */
    class MachineEditor : public juce::AudioProcessorEditor
    {
    public:
        explicit MachineEditor(MachineProcessor&);
        ~MachineEditor() override;

        void paint(juce::Graphics&) override;
        void resized() override;

    private:
        struct KnobRow
        {
            juce::String title;
            std::vector<std::unique_ptr<juce::Slider>> sliders;
            std::vector<std::unique_ptr<juce::Label>> labels;
            std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>> attachments;
        };

        void buildRow(KnobRow& row, const juce::String& title,
                      const std::vector<std::pair<juce::String, juce::String>>& idsAndNames);
        void layoutRow(KnobRow& row, juce::Rectangle<int> area);
        void paintRow(juce::Graphics&, const KnobRow& row, juce::Rectangle<int> area);

        MachineProcessor& proc;
        MDLookAndFeel lookAndFeel;

        KnobRow synRow, fltRow, ampRow;

        juce::ToggleButton kybdButton { "KYBD MODE" };
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> kybdAttachment;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MachineEditor)
    };
}
