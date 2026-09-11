#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace fmdrum
{
    FMDrumAudioProcessor::FMDrumAudioProcessor()
        : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
    {
        synth.clearVoices();
        for (int i = 0; i < numVoices; ++i)
            synth.addVoice(new FMDrumVoice(voiceParameters));

        synth.clearSounds();
        synth.addSound(new FMDrumSound());
    }

    void FMDrumAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        synth.setCurrentPlaybackSampleRate(sampleRate);

        juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), 1 };
        for (int i = 0; i < synth.getNumVoices(); ++i)
            if (auto* voice = dynamic_cast<FMDrumVoice*>(synth.getVoice(i)))
                voice->prepare(spec);
    }

    bool FMDrumAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
    {
        return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }

    void FMDrumAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        juce::ScopedNoDenormals noDenormals;
        buffer.clear();

        updateVoiceParameters(voiceParameters, apvts);

        synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    }

    juce::AudioProcessorEditor* FMDrumAudioProcessor::createEditor()
    {
        return new FMDrumAudioProcessorEditor(*this);
    }

    void FMDrumAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
    {
        if (auto state = apvts.copyState(); state.isValid())
            if (auto xml = state.createXml())
                copyXmlToBinary(*xml, destData);
    }

    void FMDrumAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
    {
        if (auto xml = getXmlFromBinary(data, sizeInBytes))
            if (xml->hasTagName(apvts.state.getType()))
                apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new fmdrum::FMDrumAudioProcessor();
}
