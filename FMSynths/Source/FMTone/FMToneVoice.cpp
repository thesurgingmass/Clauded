#include "FMToneVoice.h"
#include "Common/DspUtils.h"

namespace fmtone
{
    void FMToneVoice::prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        for (auto& op : operators)
            op.prepare(spec.sampleRate);

        ampEnvelope.setSampleRate(spec.sampleRate);
        filter.prepare(spec);
    }

    void FMToneVoice::applyParametersForCurrentNote()
    {
        for (size_t i = 0; i < operators.size(); ++i)
        {
            const auto& settings = params.operators[i];
            operators[i].setRatio(settings.ratioCoarse, settings.ratioFine);
            operators[i].setLevel(settings.level);
            operators[i].setFeedbackAmount(settings.feedback);
            operators[i].setEnvelope(settings.envelope);
        }

        ampEnvelope.setParameters(params.ampEnvelope);

        filter.setType(params.filterType);
        filter.setBaseCutoffHz(params.filterCutoffHz);
        filter.setResonance(params.filterResonance);
        filter.setEnvelopeAmount(params.filterEnvAmount);
        filter.setKeyTrackAmount(params.filterKeyTrack);
        filter.setEnvelope(params.filterEnvelope);

        drive.setAmount(params.driveAmount);
    }

    void FMToneVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*,
                                 int currentPitchWheelPosition)
    {
        baseNote = static_cast<float>(midiNoteNumber);
        currentVelocity = velocity;
        pitchWheelMoved(currentPitchWheelPosition);

        applyParametersForCurrentNote();

        for (auto& op : operators)
            op.reset();
        filter.reset();

        for (auto& op : operators)
            op.noteOn();
        ampEnvelope.noteOn();
        filter.noteOn();

        active = true;
    }

    void FMToneVoice::stopNote(float, bool allowTailOff)
    {
        if (allowTailOff)
        {
            for (auto& op : operators)
                op.noteOff();
            ampEnvelope.noteOff();
            filter.noteOff();
        }
        else
        {
            active = false;
            clearCurrentNote();
        }
    }

    void FMToneVoice::pitchWheelMoved(int newPitchWheelValue)
    {
        // newPitchWheelValue is 0..16383, centered at 8192.
        const float normalized = (static_cast<float>(newPitchWheelValue) - 8192.0f) / 8192.0f;
        pitchWheelSemitoneOffset = normalized * pitchBendRangeSemitones;
    }

    void FMToneVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
    {
        if (!active)
            return;

        applyParametersForCurrentNote();

        const auto& algorithm = getAlgorithms()[static_cast<size_t>(
            juce::jlimit(0, static_cast<int>(getAlgorithms().size()) - 1, params.algorithmIndex))];

        constexpr float modulationIndexScale = fm::defaultModulationIndexScale;

        const int numChannels = outputBuffer.getNumChannels();

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const double baseFrequency = fm::noteToFrequency(baseNote + pitchWheelSemitoneOffset);

            std::array<float, numOperators> currentOutputs {};
            float carrierSum = 0.0f;

            for (int opIndex : algorithm.processOrder)
            {
                float phaseModulation = 0.0f;
                for (const auto& connection : algorithm.connections)
                    if (connection.destination == opIndex)
                        phaseModulation += currentOutputs[static_cast<size_t>(connection.source)] * modulationIndexScale;

                const float output = operators[static_cast<size_t>(opIndex)].renderSample(baseFrequency, phaseModulation);
                currentOutputs[static_cast<size_t>(opIndex)] = output;

                if (algorithm.isCarrier[static_cast<size_t>(opIndex)])
                    carrierSum += output;
            }

            float voiceOutput = carrierSum * ampEnvelope.getNextSample() * currentVelocity;

            filter.updateCoefficients(baseNote);
            voiceOutput = filter.processSample(voiceOutput);
            voiceOutput = drive.processSample(voiceOutput);
            voiceOutput *= params.outputGain;

            for (int channel = 0; channel < numChannels; ++channel)
                outputBuffer.addSample(channel, startSample + sample, voiceOutput);
        }

        if (!ampEnvelope.isActive())
        {
            active = false;
            clearCurrentNote();
        }
    }
}
