#include "FormantOneVoice.h"
#include "Common/DspUtils.h"

namespace formantone
{
    void FormantOneVoice::prepare(double newSampleRate)
    {
        sampleRate = newSampleRate;

        for (auto& op : operators)
            op.prepare(sampleRate);
        for (auto& formant : formants)
            formant.prepare(sampleRate);

        ampEnvelope.setSampleRate(sampleRate);
        filter.prepare(sampleRate);
        lfo.prepare(sampleRate);
    }

    void FormantOneVoice::applyParametersForCurrentNote()
    {
        for (size_t i = 0; i < operators.size(); ++i)
        {
            const auto& settings = params.operators[i];
            operators[i].setRatio(settings.ratioCoarse, settings.ratioFine);
            operators[i].setFixedFrequency(settings.fixedFrequencyMode, settings.fixedFrequencyHz);
            operators[i].setLevel(settings.level);
            operators[i].setFeedbackAmount(i == 0 ? settings.feedback : 0.0f);
            operators[i].setEnvelope(settings.envelope);
        }

        for (size_t i = 0; i < formants.size(); ++i)
        {
            const auto& settings = params.formants[i];
            formants[i].setFrequency(settings.frequencyHz);
            formants[i].setBandwidth(settings.bandwidth);
            formants[i].setLevel(settings.level);
            formants[i].setVoicedUnvoiced(params.voicedUnvoiced);
            formants[i].setEnvelope(settings.envelope);
            formants[i].updateForBlock();
        }

        ampEnvelope.setParameters(params.ampEnvelope);

        filter.setType(params.filterType);
        filter.setResonance(params.filterResonance);
        filter.setEnvelopeAmount(params.filterEnvAmount);
        filter.setKeyTrackAmount(params.filterKeyTrack);
        filter.setEnvelope(params.filterEnvelope);

        lfo.setShape(params.lfo.shape);
        lfo.setRateHz(params.lfo.rateHz);
        lfo.setDelaySeconds(params.lfo.delaySeconds);

        drive.setAmount(params.driveAmount);
    }

    void FormantOneVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*,
                                     int currentPitchWheelPosition)
    {
        baseNote = static_cast<float>(midiNoteNumber);
        currentVelocity = velocity;
        pitchWheelMoved(currentPitchWheelPosition);

        applyParametersForCurrentNote();

        for (auto& op : operators)
            op.reset();
        for (auto& formant : formants)
            formant.reset();
        noise.reset();
        filter.reset();
        lfo.reset();

        for (auto& op : operators)
            op.noteOn();
        for (auto& formant : formants)
            formant.noteOn();
        ampEnvelope.noteOn();
        filter.noteOn();
        lfo.noteOn();

        active = true;
    }

    void FormantOneVoice::stopNote(float, bool allowTailOff)
    {
        if (allowTailOff)
        {
            for (auto& op : operators)
                op.noteOff();
            for (auto& formant : formants)
                formant.noteOff();
            ampEnvelope.noteOff();
            filter.noteOff();
        }
        else
        {
            active = false;
            clearCurrentNote();
        }
    }

    void FormantOneVoice::pitchWheelMoved(int newPitchWheelValue)
    {
        // newPitchWheelValue is 0..16383, centered at 8192.
        const float normalized = (static_cast<float>(newPitchWheelValue) - 8192.0f) / 8192.0f;
        pitchWheelSemitoneOffset = normalized * pitchBendRangeSemitones;
    }

    void FormantOneVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
    {
        if (!active)
            return;

        applyParametersForCurrentNote();

        const auto& algorithm = getAlgorithms()[static_cast<size_t>(
            juce::jlimit(0, static_cast<int>(getAlgorithms().size()) - 1, params.algorithmIndex))];

        constexpr float modulationIndexScale = fs::defaultModulationIndexScale;

        const int numChannels = outputBuffer.getNumChannels();

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float lfoValue = lfo.getNextSample();
            const float pitchedNote = baseNote + pitchWheelSemitoneOffset + lfoValue * params.lfo.pitchDepthSemitones;
            const double fundamentalHz = fs::noteToFrequency(pitchedNote);

            // FM engine: operators through the selected algorithm.
            std::array<float, numOperators> currentOutputs {};
            float carrierSum = 0.0f;

            for (int opIndex : algorithm.processOrder)
            {
                float phaseModulation = 0.0f;
                for (const auto& connection : algorithm.connections)
                    if (connection.destination == opIndex)
                        phaseModulation += currentOutputs[static_cast<size_t>(connection.source)] * modulationIndexScale;

                const float output = operators[static_cast<size_t>(opIndex)].renderSample(fundamentalHz, phaseModulation);
                currentOutputs[static_cast<size_t>(opIndex)] = output;

                if (algorithm.isCarrier[static_cast<size_t>(opIndex)])
                    carrierSum += output;
            }

            // Formant engine: 3 formant generators sharing one noise source.
            const float noiseSample = noise.nextSample();
            float formantSum = 0.0f;
            for (auto& formant : formants)
                formantSum += formant.renderSample(fundamentalHz, noiseSample);

            float voiceOutput = carrierSum * (1.0f - params.fmFormantBalance) + formantSum * params.fmFormantBalance;

            const float ampGain = ampEnvelope.getNextSample() * (1.0f + lfoValue * params.lfo.ampDepth * 0.5f);
            voiceOutput *= ampGain * currentVelocity;

            filter.setBaseCutoffHz(params.filterCutoffHz * std::pow(2.0f, lfoValue * params.lfo.filterDepthOctaves));
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
