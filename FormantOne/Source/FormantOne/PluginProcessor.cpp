#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace formantone
{
    FormantOneAudioProcessor::FormantOneAudioProcessor()
        : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
    {
        synth.clearVoices();
        for (int i = 0; i < numVoices; ++i)
            synth.addVoice(new FormantOneVoice(voiceParameters));

        synth.clearSounds();
        synth.addSound(new FormantOneSound());
    }

    void FormantOneAudioProcessor::prepareToPlay(double sampleRate, int)
    {
        synth.setCurrentPlaybackSampleRate(sampleRate);

        for (int i = 0; i < synth.getNumVoices(); ++i)
            if (auto* voice = dynamic_cast<FormantOneVoice*>(synth.getVoice(i)))
                voice->prepare(sampleRate);
    }

    bool FormantOneAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
    {
        return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }

    void FormantOneAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        juce::ScopedNoDenormals noDenormals;
        buffer.clear();

        updateVoiceParameters(voiceParameters, apvts);

        synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    }

    juce::AudioProcessorEditor* FormantOneAudioProcessor::createEditor()
    {
        return new FormantOneAudioProcessorEditor(*this);
    }

    void FormantOneAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
    {
        if (auto state = apvts.copyState(); state.isValid())
            if (auto xml = state.createXml())
                copyXmlToBinary(*xml, destData);
    }

    void FormantOneAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
    {
        if (auto xml = getXmlFromBinary(data, sizeInBytes))
            if (xml->hasTagName(apvts.state.getType()))
                apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new formantone::FormantOneAudioProcessor();
}
