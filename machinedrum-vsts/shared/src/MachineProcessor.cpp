#include "md/MachineProcessor.h"
#include "md/MachineEditor.h"
#include "md/dsp/Shaper.h"
#include <cmath>

namespace md
{
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

        // --- TFX page ---
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "AMD", 1 }, "AMD", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "AMF", 1 }, "AMF", juce::NormalisableRange<float>(0.1f, 30.0f, 0.0f, 0.5f), 4.0f, juce::AudioParameterFloatAttributes().withLabel("Hz")));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "EQF", 1 }, "EQF", juce::NormalisableRange<float>(60.0f, 12000.0f, 0.0f, 0.3f), 1000.0f, juce::AudioParameterFloatAttributes().withLabel("Hz")));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "EQG", 1 }, "EQG", juce::NormalisableRange<float>(-15.0f, 15.0f), 0.0f, juce::AudioParameterFloatAttributes().withLabel("dB")));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "FLTF", 1 }, "FLTF", juce::NormalisableRange<float>(20.0f, 20000.0f, 0.0f, 0.3f), 12000.0f, juce::AudioParameterFloatAttributes().withLabel("Hz")));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "FLTW", 1 }, "FLTW", juce::NormalisableRange<float>(0.0f, 8000.0f, 0.0f, 0.4f), 0.0f, juce::AudioParameterFloatAttributes().withLabel("Hz")));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "FLTQ", 1 }, "FLTQ", juce::NormalisableRange<float>(0.0f, 1.0f), 0.1f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "SRR", 1 }, "SRR", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

        // --- ROUTING page (LFOS/LFOD/LFOM omitted: LFO functionality) ---
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "DIST", 1 }, "DIST", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "VOL", 1 }, "VOL", juce::NormalisableRange<float>(0.0f, 1.0f), 1.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "PAN", 1 }, "PAN", juce::NormalisableRange<float>(-64.0f, 63.0f), 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "DEL", 1 }, "DEL", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "REV", 1 }, "REV", juce::NormalisableRange<float>(0.0f, 1.0f), 0.0f));

        // --- Physical (always-visible) track level knob ---
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { "LEV", 1 }, "LEV", juce::NormalisableRange<float>(0.0f, 1.0f), 0.85f));

        params.push_back(std::make_unique<juce::AudioParameterBool>(
            juce::ParameterID { "KYBD", 1 }, "KYBD MODE", false));

        return { params.begin(), params.end() };
    }

    void MachineProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
    {
        currentSampleRate = sampleRate;

        engine->prepare(sampleRate);
        engine->reset();

        tremolo.prepare(sampleRate);
        tremolo.reset();
        gapFilter.prepare(sampleRate);
        gapFilter.reset();
        srr.reset();

        juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 1 };
        eq.prepare(spec);
        eq.reset();
        lastEqFreq = lastEqGain = -1.0f;

        delayL.prepare(sampleRate);
        delayR.prepare(sampleRate);
        delayL.reset();
        delayR.reset();

        juce::dsp::ProcessSpec stereoSpec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
        reverb.prepare(stereoSpec);
        reverb.reset();
        reverbScratch.setSize(2, samplesPerBlock);
        juce::Reverb::Parameters rp;
        rp.roomSize = 0.6f;
        rp.damping = 0.5f;
        rp.wetLevel = 1.0f;
        rp.dryLevel = 0.0f;
        rp.width = 1.0f;
        reverb.setParameters(rp);

        synParamPtrs.clear();
        for (auto& p : machineInfo.synParams)
            synParamPtrs.push_back(apvts.getRawParameterValue(p.id));

        amd      = apvts.getRawParameterValue("AMD");
        amf      = apvts.getRawParameterValue("AMF");
        eqf      = apvts.getRawParameterValue("EQF");
        eqg      = apvts.getRawParameterValue("EQG");
        fltf     = apvts.getRawParameterValue("FLTF");
        fltw     = apvts.getRawParameterValue("FLTW");
        fltq     = apvts.getRawParameterValue("FLTQ");
        srrParam = apvts.getRawParameterValue("SRR");
        dist     = apvts.getRawParameterValue("DIST");
        vol      = apvts.getRawParameterValue("VOL");
        pan      = apvts.getRawParameterValue("PAN");
        del      = apvts.getRawParameterValue("DEL");
        rev      = apvts.getRawParameterValue("REV");
        lev      = apvts.getRawParameterValue("LEV");
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

        engine->trigger(velocity01, pitchOffset);
    }

    void MachineProcessor::renderRange(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
    {
        float amdV = amd->load(), amfV = amf->load();
        float eqfV = eqf->load(), eqgV = eqg->load();
        float fltfV = fltf->load(), fltwV = fltw->load(), fltqV = fltq->load();
        float srrV = srrParam->load();
        float distV = dist->load(), volV = vol->load(), panV = pan->load() / 64.0f;
        float delV = del->load(), revV = rev->load(), levV = lev->load();

        if (std::abs(eqfV - lastEqFreq) > 1.0f || std::abs(eqgV - lastEqGain) > 0.01f)
        {
            *eq.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(
                currentSampleRate, eqfV, 0.9f, juce::Decibels::decibelsToGain(eqgV));
            lastEqFreq = eqfV;
            lastEqGain = eqgV;
        }

        gapFilter.setParams(fltfV, fltwV, fltqV);

        auto* left  = buffer.getWritePointer(0);
        auto* right = buffer.getNumChannels() > 1 ? buffer.getWritePointer(1) : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
            float raw = engine->renderSample();

            float tremmed = tremolo.process(raw, amdV, amfV);
            float filtered = gapFilter.process(tremmed);
            float eqd = eq.processSample(filtered);
            float crushed = srr.process(eqd, srrV);
            float driven = distV > 0.0f ? softClip(crushed, distV) : crushed;

            float wet = driven * volV * levV;

            float l, r;
            equalPowerPan(wet, panV, l, r);

            float sendL = l * delV, sendR = r * delV;
            float dlyL = delayL.process(sendL);
            float dlyR = delayR.process(sendR);
            l += dlyL;
            r += dlyR;

            left[startSample + i]  = l;
            if (right != nullptr) right[startSample + i] = r;
        }

        if (revV > 0.0f && right != nullptr)
        {
            // reverbScratch is pre-sized to samplesPerBlock in prepareToPlay,
            // so this never allocates on the audio thread.
            for (int ch = 0; ch < 2; ++ch)
                juce::FloatVectorOperations::copyWithMultiply(
                    reverbScratch.getWritePointer(ch), buffer.getReadPointer(ch, startSample), revV, numSamples);

            juce::dsp::AudioBlock<float> block(reverbScratch);
            auto sub = block.getSubBlock(0, (size_t) numSamples);
            juce::dsp::ProcessContextReplacing<float> ctx(sub);
            reverb.process(ctx);

            for (int ch = 0; ch < 2; ++ch)
                buffer.addFrom(ch, startSample, reverbScratch, ch, 0, numSamples);
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
        auto state = apvts.copyState();
        std::unique_ptr<juce::XmlElement> xml(state.createXml());
        copyXmlToBinary(*xml, destData);
    }

    void MachineProcessor::setStateInformation(const void* data, int sizeInBytes)
    {
        std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
        if (xml != nullptr && xml->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xml));
    }
}
