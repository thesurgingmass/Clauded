#include "Parameters.h"

namespace fmdrum
{
    namespace ParamIDs
    {
        juce::String bodyRatio() { return "body_ratio"; }
        juce::String bodyTune() { return "body_tune"; }
        juce::String bodyIndex() { return "body_index"; }
        juce::String bodyFeedback() { return "body_feedback"; }
        juce::String bodyLevel() { return "body_level"; }

        juce::String pitchEnvAmount() { return "pitchEnv_amount"; }
        juce::String pitchEnvDecay() { return "pitchEnv_decay"; }

        juce::String noiseLevel() { return "noise_level"; }
        juce::String noiseFilterType() { return "noise_filterType"; }
        juce::String noiseFilterCutoff() { return "noise_filterCutoff"; }
        juce::String noiseAttack() { return "noise_attack"; }
        juce::String noiseDecay() { return "noise_decay"; }
        juce::String noiseSustain() { return "noise_sustain"; }
        juce::String noiseRelease() { return "noise_release"; }

        juce::String transientLevel() { return "transient_level"; }
        juce::String transientDecay() { return "transient_decay"; }

        juce::String ampAttack() { return "amp_attack"; }
        juce::String ampDecay() { return "amp_decay"; }
        juce::String ampSustain() { return "amp_sustain"; }
        juce::String ampRelease() { return "amp_release"; }

        juce::String filterType() { return "filter_type"; }
        juce::String filterCutoff() { return "filter_cutoff"; }
        juce::String filterResonance() { return "filter_resonance"; }
        juce::String filterEnvAmount() { return "filter_envAmount"; }
        juce::String filterKeyTrack() { return "filter_keyTrack"; }
        juce::String filterAttack() { return "filterEnv_attack"; }
        juce::String filterDecay() { return "filterEnv_decay"; }
        juce::String filterSustain() { return "filterEnv_sustain"; }
        juce::String filterRelease() { return "filterEnv_release"; }

        juce::String driveAmount() { return "drive_amount"; }
        juce::String outputGain() { return "output_gain"; }
    }

    namespace
    {
        juce::NormalisableRange<float> timeRangeMs(float maxMs = 5000.0f)
        {
            return { 1.0f, maxMs, 0.0f, 0.3f };
        }

        std::unique_ptr<juce::AudioParameterFloat> makeTimeParam(const juce::String& id, const juce::String& name, float defaultMs, float maxMs = 5000.0f)
        {
            return std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { id, 1 }, name, timeRangeMs(maxMs), defaultMs, "ms");
        }

        std::unique_ptr<juce::AudioParameterFloat> makeLevelParam(const juce::String& id, const juce::String& name, float defaultValue = 1.0f)
        {
            return std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { id, 1 }, name, juce::NormalisableRange<float> { 0.0f, 1.0f }, defaultValue);
        }
    }

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { ParamIDs::bodyRatio(), 1 }, "Body Ratio",
            juce::NormalisableRange<float> { 0.25f, 16.0f, 0.0f, 0.4f }, 1.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { ParamIDs::bodyTune(), 1 }, "Body Tune",
            juce::NormalisableRange<float> { -24.0f, 24.0f }, 0.0f, "semi"));
        params.push_back(makeLevelParam(ParamIDs::bodyIndex(), "Body Index", 0.5f));
        params.push_back(makeLevelParam(ParamIDs::bodyFeedback(), "Body Feedback", 0.0f));
        params.push_back(makeLevelParam(ParamIDs::bodyLevel(), "Body Level", 1.0f));

        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { ParamIDs::pitchEnvAmount(), 1 }, "Pitch Env Amount",
            juce::NormalisableRange<float> { -48.0f, 48.0f }, 24.0f, "semi"));
        params.push_back(makeTimeParam(ParamIDs::pitchEnvDecay(), "Pitch Env Decay", 40.0f, 2000.0f));

        params.push_back(makeLevelParam(ParamIDs::noiseLevel(), "Noise Level", 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { ParamIDs::noiseFilterType(), 1 }, "Noise Filter Type",
            juce::StringArray { "Low Pass", "High Pass", "Band Pass" }, 1));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { ParamIDs::noiseFilterCutoff(), 1 }, "Noise Filter Cutoff",
            juce::NormalisableRange<float> { 20.0f, 20000.0f, 0.0f, 0.25f }, 2000.0f, "Hz"));
        params.push_back(makeTimeParam(ParamIDs::noiseAttack(), "Noise Attack", 1.0f));
        params.push_back(makeTimeParam(ParamIDs::noiseDecay(), "Noise Decay", 120.0f));
        params.push_back(makeLevelParam(ParamIDs::noiseSustain(), "Noise Sustain", 0.0f));
        params.push_back(makeTimeParam(ParamIDs::noiseRelease(), "Noise Release", 50.0f));

        params.push_back(makeLevelParam(ParamIDs::transientLevel(), "Transient Level", 0.0f));
        params.push_back(makeTimeParam(ParamIDs::transientDecay(), "Transient Decay", 5.0f, 200.0f));

        params.push_back(makeTimeParam(ParamIDs::ampAttack(), "Amp Attack", 1.0f));
        params.push_back(makeTimeParam(ParamIDs::ampDecay(), "Amp Decay", 250.0f));
        params.push_back(makeLevelParam(ParamIDs::ampSustain(), "Amp Sustain", 0.0f));
        params.push_back(makeTimeParam(ParamIDs::ampRelease(), "Amp Release", 50.0f));

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { ParamIDs::filterType(), 1 }, "Filter Type",
            juce::StringArray { "Low Pass", "High Pass", "Band Pass" }, 0));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { ParamIDs::filterCutoff(), 1 }, "Filter Cutoff",
            juce::NormalisableRange<float> { 20.0f, 20000.0f, 0.0f, 0.25f }, 12000.0f, "Hz"));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { ParamIDs::filterResonance(), 1 }, "Filter Resonance",
            juce::NormalisableRange<float> { 0.1f, 20.0f, 0.0f, 0.3f }, 0.707f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { ParamIDs::filterEnvAmount(), 1 }, "Filter Env Amount",
            juce::NormalisableRange<float> { -6.0f, 6.0f }, 0.0f, "oct"));
        params.push_back(makeLevelParam(ParamIDs::filterKeyTrack(), "Filter Key Track", 0.0f));
        params.push_back(makeTimeParam(ParamIDs::filterAttack(), "Filter Env Attack", 1.0f));
        params.push_back(makeTimeParam(ParamIDs::filterDecay(), "Filter Env Decay", 200.0f));
        params.push_back(makeLevelParam(ParamIDs::filterSustain(), "Filter Env Sustain", 0.0f));
        params.push_back(makeTimeParam(ParamIDs::filterRelease(), "Filter Env Release", 50.0f));

        params.push_back(makeLevelParam(ParamIDs::driveAmount(), "Drive", 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { ParamIDs::outputGain(), 1 }, "Output Gain",
            juce::NormalisableRange<float> { -24.0f, 12.0f }, 0.0f, "dB"));

        return { params.begin(), params.end() };
    }

    void updateVoiceParameters(VoiceParameters& v, const juce::AudioProcessorValueTreeState& apvts)
    {
        auto raw = [&apvts](const juce::String& id) { return apvts.getRawParameterValue(id)->load(); };

        v.bodyRatio = static_cast<double>(raw(ParamIDs::bodyRatio()));
        v.bodyTuneSemitones = raw(ParamIDs::bodyTune());
        v.bodyIndex = raw(ParamIDs::bodyIndex());
        v.bodyFeedback = raw(ParamIDs::bodyFeedback());
        v.bodyLevel = raw(ParamIDs::bodyLevel());

        v.pitchEnvAmountSemitones = raw(ParamIDs::pitchEnvAmount());
        v.pitchEnvDecayMs = raw(ParamIDs::pitchEnvDecay());

        v.noiseLevel = raw(ParamIDs::noiseLevel());
        v.noiseFilterType = static_cast<fm::FilterType>(static_cast<int>(raw(ParamIDs::noiseFilterType())));
        v.noiseFilterCutoffHz = raw(ParamIDs::noiseFilterCutoff());
        v.noiseEnvelope.attack = raw(ParamIDs::noiseAttack()) / 1000.0f;
        v.noiseEnvelope.decay = raw(ParamIDs::noiseDecay()) / 1000.0f;
        v.noiseEnvelope.sustain = raw(ParamIDs::noiseSustain());
        v.noiseEnvelope.release = raw(ParamIDs::noiseRelease()) / 1000.0f;

        v.transientLevel = raw(ParamIDs::transientLevel());
        v.transientDecayMs = raw(ParamIDs::transientDecay());

        v.ampEnvelope.attack = raw(ParamIDs::ampAttack()) / 1000.0f;
        v.ampEnvelope.decay = raw(ParamIDs::ampDecay()) / 1000.0f;
        v.ampEnvelope.sustain = raw(ParamIDs::ampSustain());
        v.ampEnvelope.release = raw(ParamIDs::ampRelease()) / 1000.0f;

        v.filterType = static_cast<fm::FilterType>(static_cast<int>(raw(ParamIDs::filterType())));
        v.filterCutoffHz = raw(ParamIDs::filterCutoff());
        v.filterResonance = raw(ParamIDs::filterResonance());
        v.filterEnvAmount = raw(ParamIDs::filterEnvAmount());
        v.filterKeyTrack = raw(ParamIDs::filterKeyTrack());
        v.filterEnvelope.attack = raw(ParamIDs::filterAttack()) / 1000.0f;
        v.filterEnvelope.decay = raw(ParamIDs::filterDecay()) / 1000.0f;
        v.filterEnvelope.sustain = raw(ParamIDs::filterSustain());
        v.filterEnvelope.release = raw(ParamIDs::filterRelease()) / 1000.0f;

        v.driveAmount = raw(ParamIDs::driveAmount());
        v.outputGain = juce::Decibels::decibelsToGain(raw(ParamIDs::outputGain()));
    }
}
