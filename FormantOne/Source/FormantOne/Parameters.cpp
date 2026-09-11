#include "Parameters.h"

namespace formantone
{
    namespace ParamIDs
    {
        juce::String algorithm() { return "algorithm"; }

        juce::String opOwner(int opIndex) { return "op" + juce::String(opIndex + 1); }
        juce::String opRatioCoarse(int i) { return opOwner(i) + "_ratioCoarse"; }
        juce::String opRatioFine(int i) { return opOwner(i) + "_ratioFine"; }
        juce::String opFixedMode(int i) { return opOwner(i) + "_fixedMode"; }
        juce::String opFixedFreq(int i) { return opOwner(i) + "_fixedFreq"; }
        juce::String opLevel(int i) { return opOwner(i) + "_level"; }
        juce::String opFeedback() { return "op1_feedback"; }

        juce::String envelopeId(const juce::String& owner, const char* stageSuffix)
        {
            return owner + "_" + stageSuffix;
        }

        juce::String formantOwner(int f) { return "formant" + juce::String(f + 1); }
        juce::String formantFrequency(int f) { return formantOwner(f) + "_frequency"; }
        juce::String formantBandwidth(int f) { return formantOwner(f) + "_bandwidth"; }
        juce::String formantLevel(int f) { return formantOwner(f) + "_level"; }

        juce::String voicedUnvoiced() { return "voicedUnvoiced"; }
        juce::String fmFormantBalance() { return "fmFormantBalance"; }

        juce::String ampOwner() { return "amp"; }

        juce::String filterType() { return "filter_type"; }
        juce::String filterCutoff() { return "filter_cutoff"; }
        juce::String filterResonance() { return "filter_resonance"; }
        juce::String filterEnvAmount() { return "filter_envAmount"; }
        juce::String filterKeyTrack() { return "filter_keyTrack"; }
        juce::String filterEnvOwner() { return "filterEnv"; }

        juce::String lfoShape() { return "lfo_shape"; }
        juce::String lfoRate() { return "lfo_rate"; }
        juce::String lfoDelay() { return "lfo_delay"; }
        juce::String lfoPitchDepth() { return "lfo_pitchDepth"; }
        juce::String lfoAmpDepth() { return "lfo_ampDepth"; }
        juce::String lfoFilterDepth() { return "lfo_filterDepth"; }

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

        /** Adds the 7 rate/level envelope stage params for one "owner"
            (e.g. "op3", "formant1", "amp", "filterEnv"). Defaults are given
            in stage order: attack(ms), level1, decay1(ms), level2,
            decay2(ms), sustain, release(ms). */
        void addEnvelopeParams(std::vector<std::unique_ptr<juce::RangedAudioParameter>>& params,
                                const juce::String& owner, const juce::String& ownerDisplayName,
                                const std::array<float, 7>& defaults)
        {
            const auto& stages = getEnvelopeStages();
            for (size_t s = 0; s < stages.size(); ++s)
            {
                const auto& stage = stages[s];
                const juce::String id = ParamIDs::envelopeId(owner, stage.idSuffix);
                const juce::String name = ownerDisplayName + " " + stage.label;

                if (stage.isTimeParam)
                    params.push_back(makeTimeParam(id, name, defaults[s]));
                else
                    params.push_back(makeLevelParam(id, name, defaults[s]));
            }
        }
    }

    fs::RateLevelEnvelope::Parameters readEnvelopeParameters(const juce::AudioProcessorValueTreeState& apvts,
                                                              const juce::String& owner)
    {
        auto raw = [&](const char* stage) { return apvts.getRawParameterValue(ParamIDs::envelopeId(owner, stage))->load(); };

        fs::RateLevelEnvelope::Parameters p;
        p.attackSeconds = raw("attack") / 1000.0f;
        p.level1 = raw("level1");
        p.decay1Seconds = raw("decay1") / 1000.0f;
        p.level2 = raw("level2");
        p.decay2Seconds = raw("decay2") / 1000.0f;
        p.level3 = raw("sustain");
        p.releaseSeconds = raw("release") / 1000.0f;
        p.level4 = 0.0f;
        return p;
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
            const juce::String opName = "Op " + juce::String(i + 1);

            params.push_back(std::make_unique<juce::AudioParameterChoice>(
                juce::ParameterID { ParamIDs::opRatioCoarse(i), 1 }, opName + " Ratio", getRatioChoices(), 2));
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::opRatioFine(i), 1 }, opName + " Fine",
                juce::NormalisableRange<float> { -1.0f, 1.0f }, 0.0f, "semi"));
            params.push_back(std::make_unique<juce::AudioParameterBool>(
                juce::ParameterID { ParamIDs::opFixedMode(i), 1 }, opName + " Fixed Freq", false));
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::opFixedFreq(i), 1 }, opName + " Fixed Hz",
                juce::NormalisableRange<float> { 1.0f, 10000.0f, 0.0f, 0.3f }, 440.0f, "Hz"));
            params.push_back(makeLevelParam(ParamIDs::opLevel(i), opName + " Level", i == 0 ? 1.0f : 0.5f));

            addEnvelopeParams(params, ParamIDs::opOwner(i), opName, { 1.0f, 1.0f, 250.0f, 0.8f, 250.0f, 0.8f, 50.0f });
        }

        params.push_back(makeLevelParam(ParamIDs::opFeedback(), "Op 1 Feedback", 0.0f));

        const std::array<float, 3> defaultFormantHz { 700.0f, 1200.0f, 2600.0f }; // rough F1/F2/F3 vowel-formant ballpark
        for (int f = 0; f < 3; ++f)
        {
            const juce::String formantName = "Formant " + juce::String(f + 1);

            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::formantFrequency(f), 1 }, formantName + " Freq",
                juce::NormalisableRange<float> { 100.0f, 8000.0f, 0.0f, 0.3f }, defaultFormantHz[static_cast<size_t>(f)], "Hz"));
            params.push_back(std::make_unique<juce::AudioParameterFloat>(
                juce::ParameterID { ParamIDs::formantBandwidth(f), 1 }, formantName + " Bandwidth",
                juce::NormalisableRange<float> { 0.02f, 1.0f }, 0.15f));
            params.push_back(makeLevelParam(ParamIDs::formantLevel(f), formantName + " Level", 0.5f));

            addEnvelopeParams(params, ParamIDs::formantOwner(f), formantName, { 5.0f, 1.0f, 200.0f, 0.7f, 300.0f, 0.7f, 150.0f });
        }

        params.push_back(makeLevelParam(ParamIDs::voicedUnvoiced(), "Voiced/Unvoiced", 0.0f));
        params.push_back(makeLevelParam(ParamIDs::fmFormantBalance(), "FM/Formant Balance", 0.3f));

        addEnvelopeParams(params, ParamIDs::ampOwner(), "Amp", { 2.0f, 1.0f, 300.0f, 0.8f, 300.0f, 0.8f, 200.0f });

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { ParamIDs::filterType(), 1 }, "Filter Type",
            juce::StringArray { "LPF 24", "LPF 12", "HPF 12", "BPF", "BEF" }, 0));
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

        addEnvelopeParams(params, ParamIDs::filterEnvOwner(), "Filter Env", { 1.0f, 1.0f, 300.0f, 0.0f, 300.0f, 0.0f, 100.0f });

        params.push_back(std::make_unique<juce::AudioParameterChoice>(
            juce::ParameterID { ParamIDs::lfoShape(), 1 }, "LFO Shape",
            juce::StringArray { "Sine", "Triangle", "Saw", "Square", "S&H" }, 1));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { ParamIDs::lfoRate(), 1 }, "LFO Rate",
            juce::NormalisableRange<float> { 0.01f, 20.0f, 0.0f, 0.4f }, 4.0f, "Hz"));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { ParamIDs::lfoDelay(), 1 }, "LFO Delay",
            juce::NormalisableRange<float> { 0.0f, 5000.0f, 0.0f, 0.3f }, 0.0f, "ms"));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { ParamIDs::lfoPitchDepth(), 1 }, "LFO Pitch Depth",
            juce::NormalisableRange<float> { 0.0f, 12.0f }, 0.0f, "semi"));
        params.push_back(makeLevelParam(ParamIDs::lfoAmpDepth(), "LFO Amp Depth", 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID { ParamIDs::lfoFilterDepth(), 1 }, "LFO Filter Depth",
            juce::NormalisableRange<float> { -6.0f, 6.0f }, 0.0f, "oct"));

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
            op.fixedFrequencyMode = raw(ParamIDs::opFixedMode(i)) > 0.5f;
            op.fixedFrequencyHz = static_cast<double>(raw(ParamIDs::opFixedFreq(i)));
            op.level = raw(ParamIDs::opLevel(i));
            op.feedback = (i == 0) ? raw(ParamIDs::opFeedback()) : 0.0f;
            op.envelope = readEnvelopeParameters(apvts, ParamIDs::opOwner(i));
        }

        for (int f = 0; f < 3; ++f)
        {
            auto& formant = v.formants[static_cast<size_t>(f)];
            formant.frequencyHz = raw(ParamIDs::formantFrequency(f));
            formant.bandwidth = raw(ParamIDs::formantBandwidth(f));
            formant.level = raw(ParamIDs::formantLevel(f));
            formant.envelope = readEnvelopeParameters(apvts, ParamIDs::formantOwner(f));
        }

        v.voicedUnvoiced = raw(ParamIDs::voicedUnvoiced());
        v.fmFormantBalance = raw(ParamIDs::fmFormantBalance());

        v.ampEnvelope = readEnvelopeParameters(apvts, ParamIDs::ampOwner());

        v.filterType = static_cast<fs::FilterType>(static_cast<int>(raw(ParamIDs::filterType())));
        v.filterCutoffHz = raw(ParamIDs::filterCutoff());
        v.filterResonance = raw(ParamIDs::filterResonance());
        v.filterEnvAmount = raw(ParamIDs::filterEnvAmount());
        v.filterKeyTrack = raw(ParamIDs::filterKeyTrack());
        v.filterEnvelope = readEnvelopeParameters(apvts, ParamIDs::filterEnvOwner());

        v.lfo.shape = static_cast<fs::LfoShape>(static_cast<int>(raw(ParamIDs::lfoShape())));
        v.lfo.rateHz = raw(ParamIDs::lfoRate());
        v.lfo.delaySeconds = raw(ParamIDs::lfoDelay()) / 1000.0f;
        v.lfo.pitchDepthSemitones = raw(ParamIDs::lfoPitchDepth());
        v.lfo.ampDepth = raw(ParamIDs::lfoAmpDepth());
        v.lfo.filterDepthOctaves = raw(ParamIDs::lfoFilterDepth());

        v.driveAmount = raw(ParamIDs::driveAmount());
        v.outputGain = juce::Decibels::decibelsToGain(raw(ParamIDs::outputGain()));
    }
}
