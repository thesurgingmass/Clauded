#include "SynthVoice.h"
#include "SynthSound.h"
#include <cmath>

namespace vantage
{
    namespace
    {
        constexpr ModDestination oscLevelDestinations[kNumOscillators] = {
            ModDestination::Osc1Level, ModDestination::Osc2Level, ModDestination::Osc3Level
        };
        constexpr ModDestination oscPitchDestinations[kNumOscillators] = {
            ModDestination::Osc1Pitch, ModDestination::Osc2Pitch, ModDestination::Osc3Pitch
        };
        constexpr ModDestination oscPanDestinations[kNumOscillators] = {
            ModDestination::Osc1Pan, ModDestination::Osc2Pan, ModDestination::Osc3Pan
        };
        constexpr ModDestination filterCutoffDestinations[kNumFilters] = {
            ModDestination::Filter1Cutoff, ModDestination::Filter2Cutoff
        };
        constexpr ModDestination filterResonanceDestinations[kNumFilters] = {
            ModDestination::Filter1Resonance, ModDestination::Filter2Resonance
        };

        constexpr float kPitchModRangeSemitones = 2.0f;
        constexpr float kFilterModRangeHz = 8000.0f;
        constexpr float kPitchWheelRangeSemitones = 2.0f;
    }

    void SynthVoice::prepare(double newSampleRate)
    {
        sampleRate = newSampleRate;

        for (auto& osc : oscillators)
            osc.prepare(sampleRate);
        noise.prepare(sampleRate);
        for (auto& filter : filters)
            filter.prepare(sampleRate);
        for (auto& env : envelopes)
            env.prepare(sampleRate);
        for (auto& lfo : lfos)
            lfo.prepare(sampleRate);
    }

    void SynthVoice::applyStaticParameters()
    {
        for (int i = 0; i < kNumOscillators; ++i)
        {
            oscillators[static_cast<size_t>(i)].setModel(params.oscillators[static_cast<size_t>(i)].model);
            oscillators[static_cast<size_t>(i)].setWavetablePosition(params.oscillators[static_cast<size_t>(i)].wavetablePosition);
        }

        for (int i = 0; i < kNumFilters; ++i)
        {
            filters[static_cast<size_t>(i)].setModel(params.filters[static_cast<size_t>(i)].model);
            filters[static_cast<size_t>(i)].setDrive(params.filters[static_cast<size_t>(i)].drive01);
        }

        for (int i = 0; i < kNumEnvelopes; ++i)
        {
            const auto& e = params.envelopes[static_cast<size_t>(i)];
            envelopes[static_cast<size_t>(i)].setParameters(e.attackSeconds, e.decaySeconds, e.sustain01, e.releaseSeconds);
        }

        for (int i = 0; i < kNumLFOs; ++i)
        {
            lfos[static_cast<size_t>(i)].setRateHz(params.lfos[static_cast<size_t>(i)].rateHz);
            lfos[static_cast<size_t>(i)].setWaveform(params.lfos[static_cast<size_t>(i)].waveform);
        }

        for (int i = 0; i < kNumModMatrixSlots; ++i)
            modMatrix.setSlot(i, params.modMatrixSlots[static_cast<size_t>(i)]);
    }

    bool SynthVoice::canPlaySound(juce::SynthesiserSound* sound)
    {
        return dynamic_cast<SynthSound*>(sound) != nullptr;
    }

    void SynthVoice::startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound*, int currentPitchWheelPosition)
    {
        baseFrequencyHz = static_cast<float>(juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber));
        velocityGain = velocity;
        pitchWheelMoved(currentPitchWheelPosition);

        for (auto& osc : oscillators)
            osc.reset();

        for (auto& env : envelopes)
            env.noteOn();
    }

    void SynthVoice::stopNote(float, bool allowTailOff)
    {
        if (allowTailOff)
        {
            for (auto& env : envelopes)
                env.noteOff();
        }
        else
        {
            for (auto& env : envelopes)
                env.reset();
            clearCurrentNote();
        }
    }

    void SynthVoice::pitchWheelMoved(int newPitchWheelValue)
    {
        pitchWheelSemitones = (static_cast<float>(newPitchWheelValue) - 8192.0f) / 8192.0f * kPitchWheelRangeSemitones;
    }

    void SynthVoice::controllerMoved(int, int) {}

    void SynthVoice::renderOneSample(float& outLeft, float& outRight)
    {
        const float env3Value = envelopes[kFreeEnvIndex].renderSample();
        modMatrix.setSourceValue(ModSource::Env3, env3Value);

        for (int i = 0; i < kNumLFOs; ++i)
        {
            const float lfoValue = lfos[static_cast<size_t>(i)].renderSample();
            modMatrix.setSourceValue(static_cast<ModSource>(static_cast<int>(ModSource::LFO1) + i), lfoValue);
        }

        float filter1InL = 0.0f, filter1InR = 0.0f;
        float filter2InL = 0.0f, filter2InR = 0.0f;
        float bypassL = 0.0f, bypassR = 0.0f;

        for (int i = 0; i < kNumOscillators; ++i)
        {
            const auto& oscParams = params.oscillators[static_cast<size_t>(i)];

            const float level = juce::jlimit(0.0f, 1.0f,
                oscParams.level + modMatrix.getModulation(oscLevelDestinations[i]));

            const float semitoneOffset = oscParams.coarseSemitones + oscParams.fineCents * 0.01f
                + modMatrix.getModulation(oscPitchDestinations[i]) * kPitchModRangeSemitones
                + pitchWheelSemitones;
            const float freqHz = baseFrequencyHz * std::pow(2.0f, semitoneOffset / 12.0f);

            auto& osc = oscillators[static_cast<size_t>(i)];
            osc.setFrequency(freqHz);
            const float sample = osc.renderSample() * level;

            const float pan = juce::jlimit(-1.0f, 1.0f,
                oscParams.pan + modMatrix.getModulation(oscPanDestinations[i]));
            const float leftGain = std::sqrt(0.5f * (1.0f - pan));
            const float rightGain = std::sqrt(0.5f * (1.0f + pan));
            const float sampleL = sample * leftGain;
            const float sampleR = sample * rightGain;

            switch (oscParams.routing)
            {
                case OscillatorRouting::Filter1:
                    filter1InL += sampleL; filter1InR += sampleR;
                    break;
                case OscillatorRouting::Filter2:
                    filter2InL += sampleL; filter2InR += sampleR;
                    break;
                case OscillatorRouting::Both:
                    filter1InL += sampleL; filter1InR += sampleR;
                    filter2InL += sampleL; filter2InR += sampleR;
                    break;
                case OscillatorRouting::Bypass:
                case OscillatorRouting::count:
                default:
                    bypassL += sampleL; bypassR += sampleR;
                    break;
            }
        }

        const float noiseLevel = juce::jlimit(0.0f, 1.0f,
            params.noiseLevel + modMatrix.getModulation(ModDestination::NoiseLevel));
        const float noiseSample = noise.renderSample(params.noiseColor) * noiseLevel;
        filter1InL += noiseSample; filter1InR += noiseSample;
        filter2InL += noiseSample; filter2InR += noiseSample;

        const float filterEnvValue = envelopes[kFilterEnvIndex].renderSample();
        const float filterEnvOffsetHz = filterEnvValue * params.filterEnvAmount;

        for (int i = 0; i < kNumFilters; ++i)
        {
            const auto& filterParams = params.filters[static_cast<size_t>(i)];
            const float cutoffHz = filterParams.cutoffHz + filterEnvOffsetHz
                + modMatrix.getModulation(filterCutoffDestinations[i]) * kFilterModRangeHz;
            const float resonance = juce::jlimit(0.0f, 1.0f,
                filterParams.resonance01 + modMatrix.getModulation(filterResonanceDestinations[i]));

            filters[static_cast<size_t>(i)].setCutoffHz(cutoffHz);
            filters[static_cast<size_t>(i)].setResonance(resonance);
        }

        float filteredL, filteredR;
        if (params.filterRouting == FilterRouting::Series)
        {
            const float stage1L = filters[0].processSample(0, filter1InL);
            const float stage1R = filters[0].processSample(1, filter1InR);
            filteredL = filters[1].processSample(0, stage1L + filter2InL);
            filteredR = filters[1].processSample(1, stage1R + filter2InR);
        }
        else
        {
            const float stage1L = filters[0].processSample(0, filter1InL);
            const float stage1R = filters[0].processSample(1, filter1InR);
            const float stage2L = filters[1].processSample(0, filter2InL);
            const float stage2R = filters[1].processSample(1, filter2InR);
            filteredL = stage1L + stage2L;
            filteredR = stage1R + stage2R;
        }

        const float ampEnvValue = envelopes[kAmpEnvIndex].renderSample();
        const float amp = juce::jmax(0.0f, ampEnvValue + modMatrix.getModulation(ModDestination::AmpLevel));
        const float gain = amp * velocityGain * params.masterGain;

        outLeft = (filteredL + bypassL) * gain;
        outRight = (filteredR + bypassR) * gain;
    }

    void SynthVoice::renderNextBlock(juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
    {
        applyStaticParameters();

        for (int i = 0; i < numSamples; ++i)
        {
            float left = 0.0f, right = 0.0f;
            renderOneSample(left, right);

            for (int ch = 0; ch < outputBuffer.getNumChannels(); ++ch)
                outputBuffer.addSample(ch, startSample + i, ch == 0 ? left : right);

            if (!envelopes[kAmpEnvIndex].isActive())
            {
                clearCurrentNote();
                break;
            }
        }
    }
}
