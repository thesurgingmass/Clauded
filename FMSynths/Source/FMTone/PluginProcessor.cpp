#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace fmtone
{
    FMToneAudioProcessor::FMToneAudioProcessor()
        : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          apvts(*this, nullptr, "PARAMETERS", createParameterLayout())
    {
        synth.clearVoices();
        for (int i = 0; i < numVoices; ++i)
            synth.addVoice(new FMToneVoice(voiceParameters));

        synth.clearSounds();
        synth.addSound(new FMToneSound());
    }

    void FMToneAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        synth.setCurrentPlaybackSampleRate(sampleRate);

        juce::dsp::ProcessSpec spec { sampleRate, static_cast<juce::uint32>(samplesPerBlock), 1 };
        for (int i = 0; i < synth.getNumVoices(); ++i)
            if (auto* voice = dynamic_cast<FMToneVoice*>(synth.getVoice(i)))
                voice->prepare(spec);
    }

    bool FMToneAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
    {
        return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }

    void FMToneAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
    {
        juce::ScopedNoDenormals noDenormals;
        buffer.clear();

        updateVoiceParameters(voiceParameters, apvts);

        synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
    }

    juce::AudioProcessorEditor* FMToneAudioProcessor::createEditor()
    {
        return new FMToneAudioProcessorEditor(*this);
    }

    void FMToneAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
    {
        if (auto state = apvts.copyState(); state.isValid())
            if (auto xml = state.createXml())
                copyXmlToBinary(*xml, destData);
    }

    void FMToneAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
    {
        if (auto xml = getXmlFromBinary(data, sizeInBytes))
            if (xml->hasTagName(apvts.state.getType()))
                apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new fmtone::FMToneAudioProcessor();
}
