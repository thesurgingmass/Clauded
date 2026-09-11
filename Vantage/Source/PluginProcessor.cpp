#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace vantage
{
    VantageAudioProcessor::VantageAudioProcessor()
        : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
    {
        synth.clearVoices();
        for (int i = 0; i < kMaxVoices; ++i)
            synth.addVoice(new SynthVoice(voiceParameters));

        synth.clearSounds();
        synth.addSound(new SynthSound());
    }

    void VantageAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        synth.setCurrentPlaybackSampleRate(sampleRate);

        for (int i = 0; i < synth.getNumVoices(); ++i)
            if (auto* voice = dynamic_cast<SynthVoice*>(synth.getVoice(i)))
                voice->prepare(sampleRate);

        fxEngine.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
    }

    bool VantageAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
    {
        return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }

    void VantageAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        juce::ScopedNoDenormals noDenormals;
        buffer.clear();

        updateVoiceParameters(voiceParameters, apvts);
        updateGlobalParameters(globalParameters, apvts);
        fxEngine.setDelayParameters(globalParameters.delay);
        fxEngine.setReverbParameters(globalParameters.reverb);

        synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

        fxEngine.process(buffer);
    }

    juce::AudioProcessorEditor* VantageAudioProcessor::createEditor()
    {
        return new VantageAudioProcessorEditor(*this);
    }

    void VantageAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
    {
        if (auto state = apvts.copyState(); state.isValid())
            if (auto xml = state.createXml())
                copyXmlToBinary(*xml, destData);
    }

    void VantageAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
    {
        if (auto xml = getXmlFromBinary(data, sizeInBytes))
            if (xml->hasTagName(apvts.state.getType()))
                apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new vantage::VantageAudioProcessor();
}
