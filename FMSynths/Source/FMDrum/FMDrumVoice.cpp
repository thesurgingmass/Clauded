#include "FMDrumVoice.h"
#include "Common/DspUtils.h"

namespace fmdrum
{
    namespace
    {
        // op1/op2's own envelopes are held permanently "on" (no shaping of
        // their own) since amplitude for the body stage comes from the
        // shared amp envelope + bodyLevel instead - this keeps FMOperator
        // reusable as-is rather than special-casing an "unshaped" mode.
        juce::ADSR::Parameters constantOnEnvelope() { return { 0.001f, 0.001f, 1.0f, 0.001f }; }

        juce::dsp::StateVariableTPTFilterType toJuceFilterType(fm::FilterType type)
        {
            switch (type)
            {
                case fm::FilterType::highPass: return juce::dsp::StateVariableTPTFilterType::highpass;
                case fm::FilterType::bandPass: return juce::dsp::StateVariableTPTFilterType::bandpass;
                case fm::FilterType::lowPass:
                default:                       return juce::dsp::StateVariableTPTFilterType::lowpass;
            }
        }
    }

    void FMDrumVoice::prepare(const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;

        bodyOp1.prepare(spec.sampleRate);
        bodyOp2.prepare(spec.sampleRate);

        pitchEnvelope.setSampleRate(spec.sampleRate);
        noiseEnvelope.setSampleRate(spec.sampleRate);
        transientEnvelope.setSampleRate(spec.sampleRate);
        ampEnvelope.setSampleRate(spec.sampleRate);

        noiseToneFilter.prepare(spec);
        filter.prepare(spec);
    }

    void FMDrumVoice::applyParametersForCurrentNote()
    {
        bodyOp1.setRatio(1.0, 0.0);
        bodyOp1.setLevel(1.0f);
        bodyOp1.setFeedbackAmount(params.bodyFeedback);
        bodyOp1.setEnvelope(constantOnEnvelope());

        bodyOp2.setRatio(params.bodyRatio, 0.0);
        bodyOp2.setLevel(params.bodyIndex);
        bodyOp2.setFeedbackAmount(0.0f);
        bodyOp2.setEnvelope(constantOnEnvelope());

        pitchEnvelope.setParameters({ 0.001f, params.pitchEnvDecayMs / 1000.0f, 0.0f, 0.01f });

        noiseEnvelope.setParameters(params.noiseEnvelope);
        noiseToneFilter.setType(toJuceFilterType(params.noiseFilterType));
        noiseToneFilter.setCutoffFrequency(juce::jlimit(20.0f, static_cast<float>(sampleRate) * 0.49f, params.noiseFilterCutoffHz));
        noiseToneFilter.setResonance(0.707f);

        transientEnvelope.setParameters({ 0.001f, params.transientDecayMs / 1000.0f, 0.0f, 0.01f });

        ampEnvelope.setParameters(params.ampEnvelope);

        filter.setType(params.filterType);
        filter.setBaseCutoffHz(params.filterCutoffHz);
        filter.setResonance(params.filterResonance);
        filter.setEnvelopeAmount(params.filterEnvAmount);
        filter.setKeyTrackAmount(params.filterKeyTrack);
        filter.setEnvelope(params.filterEnvelope);

        drive.setAmount(params.driveAmount);
    }

    void FMDrumVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
    {
        baseNote = static_cast<float>(midiNoteNumber);
        currentVelocity = velocity;

        applyParametersForCurrentNote();

        bodyOp1.reset();
        bodyOp2.reset();
        pitchEnvelope.reset();
        noiseEnvelope.reset();
        transientEnvelope.reset();
        ampEnvelope.reset();
        noiseToneFilter.reset();
        filter.reset();

        bodyOp1.noteOn();
        bodyOp2.noteOn();
        pitchEnvelope.noteOn();
        noiseEnvelope.noteOn();
        transientEnvelope.noteOn();
        ampEnvelope.noteOn();
        filter.noteOn();

        active = true;
    }

    void FMDrumVoice::stopNote(float, bool allowTailOff)
    {
        if (allowTailOff)
        {
            // bodyOp1/bodyOp2 deliberately don't get noteOff: their own
            // envelopes are a permanent "on" gate (see constantOnEnvelope()),
            // with a short release baked in - releasing them here would cap
            // the body's audible tail at that release time regardless of the
            // user's Amp Release setting. The shared ampEnvelope below is
            // what actually shapes the release the user hears.
            noiseEnvelope.noteOff();
            transientEnvelope.noteOff();
            ampEnvelope.noteOff();
            filter.noteOff();
        }
        else
        {
            active = false;
            clearCurrentNote();
        }
    }

    void FMDrumVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
    {
        if (!active)
            return;

        applyParametersForCurrentNote();

        const int numChannels = outputBuffer.getNumChannels();

        for (int sample = 0; sample < numSamples; ++sample)
        {
            const float pitchOffset = pitchEnvelope.getNextSample() * params.pitchEnvAmountSemitones;
            const double baseFrequency = fm::noteToFrequency(baseNote + params.bodyTuneSemitones + pitchOffset);

            const float op2Output = bodyOp2.renderSample(baseFrequency, 0.0f);
            const float phaseModIntoOp1 = op2Output * fm::defaultModulationIndexScale;
            const float bodyOutput = bodyOp1.renderSample(baseFrequency, phaseModIntoOp1) * params.bodyLevel;

            float noiseSample = noiseRng.nextFloat() * 2.0f - 1.0f;
            noiseSample = noiseToneFilter.processSample(0, noiseSample);
            noiseSample *= noiseEnvelope.getNextSample() * params.noiseLevel;

            const float transientSample = (noiseRng.nextFloat() * 2.0f - 1.0f)
                                           * transientEnvelope.getNextSample() * params.transientLevel;

            float mix = bodyOutput + noiseSample + transientSample;
            mix *= ampEnvelope.getNextSample() * currentVelocity;

            filter.updateCoefficients(baseNote);
            mix = filter.processSample(mix);
            mix = drive.processSample(mix);
            mix *= params.outputGain;

            for (int channel = 0; channel < numChannels; ++channel)
                outputBuffer.addSample(channel, startSample + sample, mix);
        }

        if (!ampEnvelope.isActive())
        {
            active = false;
            clearCurrentNote();
        }
    }
}
