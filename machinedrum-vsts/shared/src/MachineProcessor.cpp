#include "md/MachineProcessor.h"
#include "md/MachineEditor.h"
#include "md/dsp/Shaper.h"
#include <cmath>

namespace md
{
    static juce::String toParamId(const juce::String& name)
    {
        return name.removeCharacters(" ").toUpperCase();
    }

    MachineProcessor::MachineProcessor(MachineInfo infoIn, std::unique_ptr<MachineEngine> engineIn)
        : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
          machineInfo(std::move(infoIn)),
          engine(std::move(engineIn)),
          apvts(*this, nullptr, "PARAMS", createParameterLayout())
    {
    }

    juce::AudioProcessorValueTreeState::ParameterLayout MachineProcessor::createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        for (auto& p : machineInfo.synParams)
        {
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { p.id, 1 }, p.name,
                juce::NormalisableRange<float>(p.minValue, p.maxValue), p.defaultValue,
                juce::AudioParameterFloatAttributes().withLabel(p.suffix)));
        }

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "FLTFREQ", 1 }, "FLT FREQ",
            juce::NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.3f), 8000.0f, juce::AudioParameterFloatAttributes().withLabel("Hz")));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "FLTRES", 1 }, "FLT RES",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.1f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "FLTATK", 1 }, "FLT ATK",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f, 0.4f), 0.0f, juce::AudioParameterFloatAttributes().withLabel("s")));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "FLTDEC", 1 }, "FLT DEC",
            juce::NormalisableRange<float>(0.005f, 2.0f, 0.0f, 0.4f), 0.15f, juce::AudioParameterFloatAttributes().withLabel("s")));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "AMPATK", 1 }, "AMP ATK",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f, 0.4f), 0.0f, juce::AudioParameterFloatAttributes().withLabel("s")));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "AMPHOLD", 1 }, "AMP HOLD",
            juce::NormalisableRange<float>(0.0f, 1.0f, 0.0f, 0.4f), 0.0f, juce::AudioParameterFloatAttributes().withLabel("s")));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "AMPDEC", 1 }, "AMP DEC",
            juce::NormalisableRange<float>(0.005f, 3.0f, 0.0f, 0.4f), 0.3f, juce::AudioParameterFloatAttributes().withLabel("s")));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "AMPVOL", 1 }, "LEVEL",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.8f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "AMPOD", 1 }, "OVERDRIVE",
            juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID { "KYBD", 1 }, "KYBD MODE", false));

        return { params.begin(), params.end() };
    }

    void MachineProcessor::prepareToPlay(double sampleRate, int)
    {
        engine->prepare(sampleRate);
        engine->reset();
        filter.prepare(sampleRate);
        filter.reset();
        filter.setType(FilterType::LowPass);
        filterEnv.prepare(sampleRate);
        ampEnv.prepare(sampleRate);

        synParamPtrs.clear();
        for (auto& p : machineInfo.synParams)
            synParamPtrs.push_back(apvts.getRawParameterValue(p.id));

        fltFreq  = apvts.getRawParameterValue("FLTFREQ");
        fltRes   = apvts.getRawParameterValue("FLTRES");
        fltAtk   = apvts.getRawParameterValue("FLTATK");
        fltDec   = apvts.getRawParameterValue("FLTDEC");
        ampAtk   = apvts.getRawParameterValue("AMPATK");
        ampHold  = apvts.getRawParameterValue("AMPHOLD");
        ampDec   = apvts.getRawParameterValue("AMPDEC");
        ampVol   = apvts.getRawParameterValue("AMPVOL");
        ampOd    = apvts.getRawParameterValue("AMPOD");
        kybdMode = apvts.getRawParameterValue("KYBD");

        synValuesCache.assign(synParamPtrs.size(), 0.0f);
    }

    bool MachineProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
    {
        return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
    }

    void MachineProcessor::handleNoteOn(int noteNumber, float velocity01)
    {
        bool kybd = kybdMode != nullptr && kybdMode->load() > 0.5f;
        float pitchOffset = kybd ? (float) (noteNumber - rootNote) : 0.0f;

        for (size_t i = 0; i < synParamPtrs.size(); ++i)
            synValuesCache[i] = synParamPtrs[i]->load();
        engine->setSynParams(synValuesCache);

        filterEnv.setTimes(fltAtk->load(), 0.0f, fltDec->load());
        filterEnv.trigger();
        ampEnv.setTimes(ampAtk->load(), ampHold->load(), ampDec->load());
        ampEnv.trigger();

        engine->trigger(velocity01, pitchOffset);
    }

    void MachineProcessor::renderRange(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
    {
        float freqHz = fltFreq->load();
        float res    = fltRes->load();
        float vol    = ampVol->load();
        float od     = ampOd->load();

        auto* left  = buffer.getWritePointer(0);
        auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
            float raw = engine->renderSample();

            float envAmt = filterEnv.process();
            float modFreq = juce::jlimit(20.0f, 20000.0f, freqHz * std::pow(2.0f, envAmt * 3.0f));
            filter.setParams(modFreq, res);
            float filtered = filter.process(raw);

            float amp = ampEnv.process();
            float wet = filtered * amp * vol;
            if (od > 0.0f)
                wet = softClip(wet, od);

            left[startSample + i] = wet;
            if (right != nullptr)
                right[startSample + i] = wet;
        }
    }

    void MachineProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
    {
        juce::ScopedNoDenormals noDenormals;
        buffer.clear();

        int lastSample = 0;
        for (const auto meta : midi)
        {
            auto msg = meta.getMessage();
            int samplePos = meta.samplePosition;

            if (samplePos > lastSample)
                renderRange(buffer, lastSample, samplePos - lastSample);

            if (msg.isNoteOn())
                handleNoteOn(msg.getNoteNumber(), msg.getFloatVelocity());

            lastSample = samplePos;
        }

        if (lastSample < buffer.getNumSamples())
            renderRange(buffer, lastSample, buffer.getNumSamples() - lastSample);
    }

    juce::AudioProcessorEditor* MachineProcessor::createEditor()
    {
        return new MachineEditor(*this);
    }

    void MachineProcessor::getStateInformation(juce::MemoryBlock& destData)
    {
        if (auto state = apvts.copyState(); true)
        {
            std::unique_ptr<juce::XmlElement> xml(state.createXml());
            copyXmlToBinary(*xml, destData);
        }
    }

    void MachineProcessor::setStateInformation(const void* data, int sizeInBytes)
    {
        std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
        if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}
