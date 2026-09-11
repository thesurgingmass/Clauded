#include "Parameters.h"

namespace fmtone
{
    namespace ParamIDs
    {
        juce::String algorithm() { return "algorithm"; }
        juce::String opRatioCoarse(int i) { return juce::String("op") + operatorNames[i] + "_ratioCoarse"; }
        juce::String opRatioFine(int i) { return juce::String("op") + operatorNames[i] + "_ratioFine"; }
        juce::String opLevel(int i) { return juce::String("op") + operatorNames[i] + "_level"; }
        juce::String opFeedback() { return "opC_feedback"; }
        juce::String opAttack(int i) { return juce::String("op") + operatorNames[i] + "_attack"; }
        juce::String opDecay(int i) { return juce::String("op") + operatorNames[i] + "_decay"; }
        juce::String opSustain(int i) { return juce::String("op") + operatorNames[i] + "_sustain"; }
        juce::String opRelease(int i) { return juce::String("op") + operatorNames[i] + "_release"; }

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
        juce::NormalisableRange<float> timeRangeMs()
        {
            return { 1.0f, 5000.0f, 0.0f, 0.3f };
        }

        std::unique_ptr<juce::AudioParameterFloat> makeTimeParam(const juce::String& id, const juce::String& name, float defaultMs)
        {
            return std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { id, 1 }, name, timeRangeMs(), defaultMs, "ms");
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

        juce::StringArray algoNames;
        for (const auto& algo : getAlgorithms())
            algoNames.add(algo.name);
        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { ParamIDs::algorithm(), 1 }, "Algorithm", algoNames, 0));

        for (int i = 0; i < numOperators; ++i)
        {
            const juce::String opName = juce::String("Op ") + operatorNames[i];

            params.push_back(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID { ParamIDs::opRatioCoarse(i), 1 }, opName + " Ratio", getRatioChoices(), 2));

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::opRatioFine(i), 1 }, opName + " Fine",
                juce::NormalisableRange<float> { -1.0f, 1.0f }, 0.0f, "semi"));

            params.push_back(makeLevelParam(ParamIDs::opLevel(i), opName + " Level", i == opC ? 1.0f : 0.5f));

            params.push_back(makeTimeParam(ParamIDs::opAttack(i), opName + " Env Attack", 1.0f));
            params.push_back(makeTimeParam(ParamIDs::opDecay(i), opName + " Env Decay", 250.0f));
            params.push_back(makeLevelParam(ParamIDs::opSustain(i), opName + " Env End Level", 1.0f));
            params.push_back(makeTimeParam(ParamIDs::opRelease(i), opName + " Env Release", 50.0f));
        }

        params.push_back(makeLevelParam(ParamIDs::opFeedback(), "Op C Feedback", 0.0f));

        params.push_back(makeTimeParam(ParamIDs::ampAttack(), "Amp Attack", 2.0f));
        params.push_back(makeTimeParam(ParamIDs::ampDecay(), "Amp Decay", 300.0f));
        params.push_back(makeLevelParam(ParamIDs::ampSustain(), "Amp Sustain", 0.8f));
        params.push_back(makeTimeParam(ParamIDs::ampRelease(), "Amp Release", 200.0f));

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
        params.push_back(makeTimeParam(ParamIDs::filterDecay(), "Filter Env Decay", 300.0f));
        params.push_back(makeLevelParam(ParamIDs::filterSustain(), "Filter Env Sustain", 0.0f));
        params.push_back(makeTimeParam(ParamIDs::filterRelease(), "Filter Env Release", 100.0f));

        params.push_back(makeLevelParam(ParamIDs::driveAmount(), "Drive", 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { ParamIDs::outputGain(), 1 }, "Output Gain",
            juce::NormalisableRange<float> { -24.0f, 12.0f }, 0.0f, "dB"));

        return { params.begin(), params.end() };
    }

    void updateVoiceParameters(VoiceParameters& v, const juce::AudioProcessorValueTreeState& apvts)
    {
        auto raw = [&apvts](const juce::String& id) { return apvts.getRawParameterValue(id)->load(); };

        v.algorithmIndex = static_cast<int>(raw(ParamIDs::algorithm()));

        for (int i = 0; i < numOperators; ++i)
        {
            auto& op = v.operators[static_cast<size_t>(i)];
            op.ratioCoarse = ratioChoiceToValue(static_cast<int>(raw(ParamIDs::opRatioCoarse(i))));
            op.ratioFine = static_cast<double>(raw(ParamIDs::opRatioFine(i)));
            op.level = raw(ParamIDs::opLevel(i));
            op.feedback = (i == opC) ? raw(ParamIDs::opFeedback()) : 0.0f;
            op.envelope.attack = raw(ParamIDs::opAttack(i)) / 1000.0f;
            op.envelope.decay = raw(ParamIDs::opDecay(i)) / 1000.0f;
            op.envelope.sustain = raw(ParamIDs::opSustain(i));
            op.envelope.release = raw(ParamIDs::opRelease(i)) / 1000.0f;
        }

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
