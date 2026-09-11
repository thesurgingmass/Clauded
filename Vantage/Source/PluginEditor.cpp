#include "PluginEditor.h"

namespace vantage
{
    VantageAudioProcessorEditor::VantageAudioProcessorEditor(VantageAudioProcessor& processorToEdit)
        : AudioProcessorEditor(&processorToEdit), processorRef(processorToEdit)
    {
        addAndMakeVisible(genericEditor);
        setResizable(true, true);
        setSize(500, 700);
    }

    void VantageAudioProcessorEditor::resized()
    {
        genericEditor.setBounds(getLocalBounds());
    }
}
