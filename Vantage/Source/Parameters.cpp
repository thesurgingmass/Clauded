#include "Parameters.h"

namespace vantage
{
    namespace ParamIDs
    {
        juce::String oscModel(int i) { return "osc" + juce::String(i + 1) + "_model"; }
        juce::String oscLevel(int i) { return "osc" + juce::String(i + 1) + "_level"; }
        juce::String oscCoarse(int i) { return "osc" + juce::String(i + 1) + "_coarse"; }
        juce::String oscFine(int i) { return "osc" + juce::String(i + 1) + "_fine"; }
        juce::String oscPan(int i) { return "osc" + juce::String(i + 1) + "_pan"; }
        juce::String oscWavetablePosition(int i) { return "osc" + juce::String(i + 1) + "_wtPos"; }
        juce::String oscRouting(int i) { return "osc" + juce::String(i + 1) + "_routing"; }

        juce::String noiseColor() { return "noise_color"; }
        juce::String noiseLevel() { return "noise_level"; }

        juce::String filterModel(int i) { return "filter" + juce::String(i + 1) + "_model"; }
        juce::String filterCutoff(int i) { return "filter" + juce::String(i + 1) + "_cutoff"; }
        juce::String filterResonance(int i) { return "filter" + juce::String(i + 1) + "_resonance"; }
        juce::String filterDrive(int i) { return "filter" + juce::String(i + 1) + "_drive"; }
        juce::String filterRouting() { return "filter_routing"; }

        juce::String envAttack(int i) { return "env" + juce::String(i + 1) + "_attack"; }
        juce::String envDecay(int i) { return "env" + juce::String(i + 1) + "_decay"; }
        juce::String envSustain(int i) { return "env" + juce::String(i + 1) + "_sustain"; }
        juce::String envRelease(int i) { return "env" + juce::String(i + 1) + "_release"; }
        juce::String filterEnvAmount() { return "env2_filterAmount"; }

        juce::String lfoRate(int i) { return "lfo" + juce::String(i + 1) + "_rate"; }
        juce::String lfoWaveform(int i) { return "lfo" + juce::String(i + 1) + "_waveform"; }

        juce::String modSource(int i) { return "mod" + juce::String(i + 1) + "_source"; }
        juce::String modDestination(int i) { return "mod" + juce::String(i + 1) + "_destination"; }
        juce::String modDepth(int i) { return "mod" + juce::String(i + 1) + "_depth"; }

        juce::String delayTime() { return "delay_time"; }
        juce::String delayFeedback() { return "delay_feedback"; }
        juce::String delayWowRate() { return "delay_wowRate"; }
        juce::String delayWowDepth() { return "delay_wowDepth"; }
        juce::String delayFlutterRate() { return "delay_flutterRate"; }
        juce::String delayFlutterDepth() { return "delay_flutterDepth"; }
        juce::String delayTone() { return "delay_tone"; }
        juce::String delayMix() { return "delay_mix"; }

        juce::String reverbSize() { return "reverb_size"; }
        juce::String reverbDecay() { return "reverb_decay"; }
        juce::String reverbMix() { return "reverb_mix"; }
        juce::String reverbDiffusion() { return "reverb_diffusion"; }
        juce::String reverbModDepth() { return "reverb_modDepth"; }
        juce::String reverbModRate() { return "reverb_modRate"; }
        juce::String reverbLowCut() { return "reverb_lowCut"; }
        juce::String reverbHighCut() { return "reverb_highCut"; }

        juce::String masterGain() { return "master_gain"; }
        juce::String polyphonyMode() { return "global_polyphonyMode"; }
        juce::String glideTimeMs() { return "global_glideTime"; }
        juce::String legato() { return "global_legato"; }
    }

    namespace
    {
        using Choice = juce::AudioParameterChoice;
        using Float = juce::AudioParameterFloat;
        using Bool = juce::AudioParameterBool;

        std::unique_ptr<Float> makeParam(const juce::String& id, const juce::String& name,
            juce::NormalisableRange<float> range, float defaultValue, const juce::String& unit = {})
        {
            return std::make_unique<Float>(juce::ParameterID { id, 1 }, name, range, defaultValue, unit);
        }

        std::unique_ptr<Float> make01Param(const juce::String& id, const juce::String& name, float defaultValue)
        {
            return makeParam(id, name, { 0.0f, 1.0f }, defaultValue);
        }

        std::unique_ptr<Float> makeBipolarParam(const juce::String& id, const juce::String& name, float defaultValue = 0.0f)
        {
            return makeParam(id, name, { -1.0f, 1.0f }, defaultValue);
        }

        std::unique_ptr<Float> makeHzParam(const juce::String& id, const juce::String& name, float minHz, float maxHz, float defaultHz)
        {
            return makeParam(id, name, { minHz, maxHz, 0.0f, 0.3f }, defaultHz, "Hz");
        }

        std::unique_ptr<Float> makeMsParam(const juce::String& id, const juce::String& name, float minMs, float maxMs, float defaultMs)
        {
            return makeParam(id, name, { minMs, maxMs, 0.0f, 0.3f }, defaultMs, "ms");
        }

        std::unique_ptr<Choice> makeChoiceParam(const juce::String& id, const juce::String& name,
            const juce::StringArray& choices, int defaultIndex)
        {
            return std::make_unique<Choice>(juce::ParameterID { id, 1 }, name, choices, defaultIndex);
        }
    }

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
    {
        std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

        for (int i = 0; i < kNumOscillators; ++i)
        {
            const juce::String name = "Osc " + juce::String(i + 1);
            params.push_back(makeChoiceParam(ParamIDs::oscModel(i), name + " Model", oscillatorModelChoices(), 0));
            params.push_back(make01Param(ParamIDs::oscLevel(i), name + " Level", i == 0 ? 0.8f : 0.0f));
            params.push_back(makeParam(ParamIDs::oscCoarse(i), name + " Coarse", { -24.0f, 24.0f, 1.0f }, 0.0f, "semi"));
            params.push_back(makeParam(ParamIDs::oscFine(i), name + " Fine", { -100.0f, 100.0f }, 0.0f, "cents"));
            params.push_back(makeBipolarParam(ParamIDs::oscPan(i), name + " Pan", 0.0f));
            params.push_back(make01Param(ParamIDs::oscWavetablePosition(i), name + " Wavetable Position", 0.0f));
            params.push_back(makeChoiceParam(ParamIDs::oscRouting(i), name + " Routing", oscillatorRoutingChoices(), 2));
        }

        params.push_back(make01Param(ParamIDs::noiseColor(), "Noise Color", 0.5f));
        params.push_back(make01Param(ParamIDs::noiseLevel(), "Noise Level", 0.0f));

        for (int i = 0; i < kNumFilters; ++i)
        {
            const juce::String name = "Filter " + juce::String(i + 1);
            params.push_back(makeChoiceParam(ParamIDs::filterModel(i), name + " Model", filterModelChoices(), 0));
            params.push_back(makeHzParam(ParamIDs::filterCutoff(i), name + " Cutoff", 20.0f, 20000.0f, i == 0 ? 8000.0f : 12000.0f));
            params.push_back(make01Param(ParamIDs::filterResonance(i), name + " Resonance", 0.2f));
            params.push_back(make01Param(ParamIDs::filterDrive(i), name + " Drive", 0.0f));
        }
        params.push_back(makeChoiceParam(ParamIDs::filterRouting(), "Filter Routing", filterRoutingChoices(), 0));

        const std::array<const char*, kNumEnvelopes> envNames { "Env 1 (Amp)", "Env 2 (Filter)", "Env 3 (Free)" };
        const std::array<float, kNumEnvelopes> envDefaultSustain { 0.7f, 0.0f, 0.5f };
        for (int i = 0; i < kNumEnvelopes; ++i)
        {
            const juce::String name = envNames[static_cast<size_t>(i)];
            params.push_back(makeMsParam(ParamIDs::envAttack(i), name + " Attack", 1.0f, 10000.0f, 5.0f));
            params.push_back(makeMsParam(ParamIDs::envDecay(i), name + " Decay", 1.0f, 10000.0f, 300.0f));
            params.push_back(make01Param(ParamIDs::envSustain(i), name + " Sustain", envDefaultSustain[static_cast<size_t>(i)]));
            params.push_back(makeMsParam(ParamIDs::envRelease(i), name + " Release", 1.0f, 10000.0f, 200.0f));
        }
        params.push_back(makeParam(ParamIDs::filterEnvAmount(), "Env 2 -> Filter Amount", { -8000.0f, 8000.0f }, 4000.0f, "Hz"));

        for (int i = 0; i < kNumLFOs; ++i)
        {
            const juce::String name = "LFO " + juce::String(i + 1);
            params.push_back(makeHzParam(ParamIDs::lfoRate(i), name + " Rate", 0.01f, 20.0f, 1.0f));
            params.push_back(makeChoiceParam(ParamIDs::lfoWaveform(i), name + " Waveform", lfoWaveformChoices(), 0));
        }

        for (int i = 0; i < kNumModMatrixSlots; ++i)
        {
            const juce::String name = "Mod " + juce::String(i + 1);
            params.push_back(makeChoiceParam(ParamIDs::modSource(i), name + " Source", modSourceChoices(), 0));
            params.push_back(makeChoiceParam(ParamIDs::modDestination(i), name + " Destination", modDestinationChoices(), 0));
            params.push_back(makeBipolarParam(ParamIDs::modDepth(i), name + " Depth", 0.0f));
        }

        params.push_back(makeMsParam(ParamIDs::delayTime(), "Delay Time", 1.0f, 2000.0f, 350.0f));
        params.push_back(make01Param(ParamIDs::delayFeedback(), "Delay Feedback", 0.35f));
        params.push_back(makeHzParam(ParamIDs::delayWowRate(), "Delay Wow Rate", 0.05f, 5.0f, 0.8f));
        params.push_back(make01Param(ParamIDs::delayWowDepth(), "Delay Wow Depth", 0.0f));
        params.push_back(makeHzParam(ParamIDs::delayFlutterRate(), "Delay Flutter Rate", 1.0f, 15.0f, 6.0f));
        params.push_back(make01Param(ParamIDs::delayFlutterDepth(), "Delay Flutter Depth", 0.0f));
        params.push_back(makeHzParam(ParamIDs::delayTone(), "Delay Tone", 200.0f, 12000.0f, 4000.0f));
        params.push_back(make01Param(ParamIDs::delayMix(), "Delay Mix", 0.0f));

        params.push_back(make01Param(ParamIDs::reverbSize(), "Reverb Size", 0.5f));
        params.push_back(make01Param(ParamIDs::reverbDecay(), "Reverb Decay", 0.5f));
        params.push_back(make01Param(ParamIDs::reverbMix(), "Reverb Mix", 0.0f));
        params.push_back(make01Param(ParamIDs::reverbDiffusion(), "Reverb Diffusion", 0.5f));
        params.push_back(make01Param(ParamIDs::reverbModDepth(), "Reverb Mod Depth", 0.2f));
        params.push_back(makeHzParam(ParamIDs::reverbModRate(), "Reverb Mod Rate", 0.01f, 5.0f, 0.3f));
        params.push_back(makeHzParam(ParamIDs::reverbLowCut(), "Reverb Low Cut", 20.0f, 2000.0f, 100.0f));
        params.push_back(makeHzParam(ParamIDs::reverbHighCut(), "Reverb High Cut", 1000.0f, 20000.0f, 12000.0f));

        params.push_back(make01Param(ParamIDs::masterGain(), "Master Gain", 0.8f));
        params.push_back(makeChoiceParam(ParamIDs::polyphonyMode(), "Polyphony Mode", polyphonyModeChoices(), 1));
        params.push_back(makeMsParam(ParamIDs::glideTimeMs(), "Glide Time", 0.0f, 2000.0f, 0.0f));
        params.push_back(std::make_unique<Bool>(juce::ParameterID { ParamIDs::legato(), 1 }, "Legato", false));

        return { params.begin(), params.end() };
    }

    void updateVoiceParameters(VoiceParameters& v, const juce::AudioProcessorValueTreeState& apvts)
    {
        auto raw = [&apvts](const juce::String& id) { return apvts.getRawParameterValue(id)->load(); };

        for (int i = 0; i < kNumOscillators; ++i)
        {
            auto& o = v.oscillators[static_cast<size_t>(i)];
            o.model = static_cast<OscillatorModel>(static_cast<int>(raw(ParamIDs::oscModel(i))));
            o.level = raw(ParamIDs::oscLevel(i));
            o.coarseSemitones = raw(ParamIDs::oscCoarse(i));
            o.fineCents = raw(ParamIDs::oscFine(i));
            o.pan = raw(ParamIDs::oscPan(i));
            o.wavetablePosition = raw(ParamIDs::oscWavetablePosition(i));
            o.routing = static_cast<OscillatorRouting>(static_cast<int>(raw(ParamIDs::oscRouting(i))));
        }

        v.noiseColor = raw(ParamIDs::noiseColor());
        v.noiseLevel = raw(ParamIDs::noiseLevel());

        for (int i = 0; i < kNumFilters; ++i)
        {
            auto& f = v.filters[static_cast<size_t>(i)];
            f.model = static_cast<FilterModel>(static_cast<int>(raw(ParamIDs::filterModel(i))));
            f.cutoffHz = raw(ParamIDs::filterCutoff(i));
            f.resonance01 = raw(ParamIDs::filterResonance(i));
            f.drive01 = raw(ParamIDs::filterDrive(i));
        }
        v.filterRouting = static_cast<FilterRouting>(static_cast<int>(raw(ParamIDs::filterRouting())));

        for (int i = 0; i < kNumEnvelopes; ++i)
        {
            auto& e = v.envelopes[static_cast<size_t>(i)];
            e.attackSeconds = raw(ParamIDs::envAttack(i)) / 1000.0f;
            e.decaySeconds = raw(ParamIDs::envDecay(i)) / 1000.0f;
            e.sustain01 = raw(ParamIDs::envSustain(i));
            e.releaseSeconds = raw(ParamIDs::envRelease(i)) / 1000.0f;
        }
        v.filterEnvAmount = raw(ParamIDs::filterEnvAmount());

        for (int i = 0; i < kNumLFOs; ++i)
        {
            auto& l = v.lfos[static_cast<size_t>(i)];
            l.rateHz = raw(ParamIDs::lfoRate(i));
            l.waveform = static_cast<LFOWaveform>(static_cast<int>(raw(ParamIDs::lfoWaveform(i))));
        }

        for (int i = 0; i < kNumModMatrixSlots; ++i)
        {
            auto& slot = v.modMatrixSlots[static_cast<size_t>(i)];
            slot.source = static_cast<ModSource>(static_cast<int>(raw(ParamIDs::modSource(i))));
            slot.destination = static_cast<ModDestination>(static_cast<int>(raw(ParamIDs::modDestination(i))));
            slot.depth = raw(ParamIDs::modDepth(i));
        }

        v.masterGain = raw(ParamIDs::masterGain());
        v.glideTimeMs = raw(ParamIDs::glideTimeMs());
    }

    void updateGlobalParameters(GlobalParameters& g, const juce::AudioProcessorValueTreeState& apvts)
    {
        auto raw = [&apvts](const juce::String& id) { return apvts.getRawParameterValue(id)->load(); };

        g.delay.timeMs = raw(ParamIDs::delayTime());
        g.delay.feedback01 = raw(ParamIDs::delayFeedback());
        g.delay.wowRateHz = raw(ParamIDs::delayWowRate());
        g.delay.wowDepth01 = raw(ParamIDs::delayWowDepth());
        g.delay.flutterRateHz = raw(ParamIDs::delayFlutterRate());
        g.delay.flutterDepth01 = raw(ParamIDs::delayFlutterDepth());
        g.delay.toneHz = raw(ParamIDs::delayTone());
        g.delay.mix01 = raw(ParamIDs::delayMix());

        g.reverb.size01 = raw(ParamIDs::reverbSize());
        g.reverb.decay01 = raw(ParamIDs::reverbDecay());
        g.reverb.mix01 = raw(ParamIDs::reverbMix());
        g.reverb.diffusion01 = raw(ParamIDs::reverbDiffusion());
        g.reverb.modDepth01 = raw(ParamIDs::reverbModDepth());
        g.reverb.modRateHz = raw(ParamIDs::reverbModRate());
        g.reverb.lowCutHz = raw(ParamIDs::reverbLowCut());
        g.reverb.highCutHz = raw(ParamIDs::reverbHighCut());

        g.masterGain = raw(ParamIDs::masterGain());
        g.polyphonyMode = static_cast<PolyphonyMode>(static_cast<int>(raw(ParamIDs::polyphonyMode())));
        g.legato = raw(ParamIDs::legato()) > 0.5f;
    }
}
