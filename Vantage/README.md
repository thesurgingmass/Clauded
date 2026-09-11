# Vantage

A flagship hybrid virtual-analog / wavetable VST3 synthesizer, spiritually
based on the Yamaha AN1x and expanded with multi-model DSP engines, dual
filters, a 6-LFO/3-envelope modulation matrix, and tape delay + spatial
reverb sends. Built with [JUCE](https://juce.com) and CMake.

> **Not a Yamaha, Moog, Oberheim, Roland, or Nord product.** "Moog",
> "Oberheim", "Roland", and "Nord" here name the *character* each
> oscillator/filter slot is modeled after (ladder/transistor, state-variable,
> OTA, digital-VA respectively) — an independent, from-scratch
> implementation, not licensed code, assets, or a bit-exact clone of any of
> those companies' products. All are trademarks of their respective owners;
> no affiliation is implied.

## Status: scaffolding (Step 1-2 of the build plan)

This pass lays down the full plugin skeleton so the audio graph compiles and
passes real audio end-to-end, without yet implementing the per-manufacturer
analog-modeling algorithms or the SH-101-style envelope curves — those are
explicitly deferred to the next phase. Concretely, right now:

- **Oscillators/filters render a working placeholder** (band-unlimited sine;
  a state-variable lowpass) regardless of which model is selected — the
  `OscillatorModel`/`FilterModel` enum and per-slot selection already flow
  end to end, so swapping in the real Moog/Oberheim/Roland/Nord DSP later is
  a matter of branching inside `OscillatorEngine::renderSample()` /
  `FilterEngine::processSample()`, not a structural change.
- **Envelopes are a standard `juce::ADSR`** — the SH-101 snap-curve shaping
  is a later pass on top of the same `EnvelopeEngine` interface.
- **The reverb runs `juce::dsp::Reverb`** as a stand-in for the eventual
  Supermassive-style granular/delay-network algorithm; `diffusion`, the two
  mod parameters, and the low/high cuts already exist as parameters but
  aren't wired into the placeholder algorithm yet.
- **The UI is JUCE's built-in generic parameter editor** (every parameter as
  an auto-generated slider/combo box) — replaced by the custom,
  decluttered, Surge-XT-inspired UI in a later phase.
- **Noise, the modulation matrix, tape delay wow/flutter, and voice
  routing/mixing are fully implemented** — these are ordinary DSP rather
  than per-manufacturer modeling, so there was no reason to stub them.

## Architecture

```
Vantage/
  CMakeLists.txt        Top-level: fetches JUCE, macOS universal-binary defaults.
  Source/
    CMakeLists.txt       juce_add_plugin(Vantage), FORMATS VST3 + Standalone.
    PluginProcessor.*     juce::AudioProcessor: APVTS, 16-voice juce::Synthesiser, FXEngine.
    PluginEditor.*         Generic APVTS editor (placeholder, see Status above).
    Parameters.*           Full APVTS ParameterLayout + APVTS -> VoiceParameters/GlobalParameters readout.
    DSP/
      Constants.h/.cpp      Shared enums (models, routings, mod sources/destinations) + combo-box choice lists.
      VoiceParameters.h      Plain-data per-block parameter snapshot (no APVTS dependency) shared by every voice.
      OscillatorEngine.*      One oscillator slot.
      NoiseGenerator.*        Continuous white -> pink -> brown morph noise source.
      FilterEngine.*          One filter slot (stereo).
      EnvelopeEngine.*        ADSR wrapper.
      LFOEngine.*             Multi-waveform free-running LFO.
      ModulationMatrix.*      16-slot source -> destination -> depth matrix.
      SynthVoice.*/SynthSound.h  Glues 3 oscillators + noise + 2 filters + 3 envelopes + 6 LFOs + a
                                  per-voice modulation matrix into a juce::SynthesiserVoice.
      FXEngine.*              Tape delay (wow/flutter/feedback/tone) into the reverb placeholder.
```

**Parameter flow**: `PluginProcessor::processBlock` calls
`updateVoiceParameters`/`updateGlobalParameters` once per block to refresh
two plain-data structs from the APVTS (`apvts.getRawParameterValue(id)`, no
audio-thread allocation). Every `SynthVoice` holds a `const VoiceParameters&`
reference to the processor's shared struct — no per-voice pushing needed —
and reads it directly while rendering. `GlobalParameters` feeds `FXEngine`.

**Modulation**: each voice owns its own 6 LFOs, ENV 3, and modulation
matrix, so LFO phase and ENV 3 modulation are independent per note (as most
polysynths do it) rather than shared/global. Every matrix slot maps one
`ModSource` (LFO 1-6 or ENV 3) to one `ModDestination` at a bipolar depth;
a destination's modulation is the sum of `depth * sourceValue` across every
slot targeting it, so several sources can stack on one destination. The
`ModDestination` enum currently covers oscillator level/pitch/pan, noise
level, both filters' cutoff/resonance, and amp level — deliberately
structured so more destinations (as more encoders become assignable in
later UI phases) are additive, not a format change.

**Signal path**: per voice, per sample — 3 oscillators (each routable to
Filter 1, Filter 2, Both, or Bypass) plus the noise generator (routed into
both filters) sum into Filter 1 and/or Filter 2's input; the two filters
combine in series (Filter 1 -> Filter 2, with a hard-wired `Bypass` signal
skipping both) or parallel (summed) per the `filter_routing` parameter;
ENV 2 adds a hard-wired cutoff offset to both filters; ENV 1 (amp) shapes
the final level. All active voices sum in `juce::Synthesiser`, then the
mix runs through `FXEngine` (tape delay into reverb).

## Parameter tree (APVTS)

| Group | Count | Per-item parameters |
|---|---|---|
| Oscillators | 3 | Model, Level, Coarse (±24 semi), Fine (±100 cents), Pan, Wavetable Position, Routing (Filter 1/Filter 2/Both/Bypass) |
| Noise | 1 | Color (white↔pink↔brown), Level |
| Filters | 2 | Model, Cutoff (20 Hz-20 kHz), Resonance, Drive |
| Filter Routing | 1 | Series / Parallel |
| Envelopes | 3 | Attack, Decay, Sustain, Release (Env 1 = amp, Env 2 = filter, both hard-wired; Env 3 = free, matrix-only) |
| Env 2 -> Filter Amount | 1 | Bipolar Hz offset applied to both filters |
| LFOs | 6 | Rate (0.01-20 Hz), Waveform (Sine/Triangle/Saw/Square/S&H) |
| Modulation Matrix | 16 slots | Source (None/LFO 1-6/Env 3), Destination (None + 21 destinations), Depth (bipolar) |
| Delay | 1 | Time, Feedback, Wow Rate/Depth, Flutter Rate/Depth, Tone, Mix |
| Reverb | 1 | Size, Decay, Mix, Diffusion, Mod Depth, Mod Rate, Low Cut, High Cut |
| Global | 1 | Master Gain, Polyphony Mode (Mono/Poly), Glide Time, Legato |

That's 3×7 + 2 + 2×4 + 1 + 3×4 + 1 + 6×2 + 16×3 + 8 + 8 + 4 = **125
parameters**, all host-automatable and saved/restored via
`AudioProcessorValueTreeState::copyState()`/`replaceState()`.

Mono/legato/glide and per-voice-vs-global LFO behavior are parsed into
`GlobalParameters` already but not yet applied in the voice-stealing logic
(`juce::Synthesiser` currently runs simple 16-voice polyphony regardless of
`polyphonyMode`) — that's next-phase work, tracked alongside the analog
modeling itself.

## Building

Requires CMake 3.22+ and a C++20 compiler. JUCE itself is fetched
automatically (`FetchContent`, pinned to release 8.0.15) — no manual JUCE
install needed.

```sh
cd Vantage
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target Vantage_VST3 --config Release --parallel
```

Resulting plugin lands in:

```
build/Source/Vantage_artefacts/Release/VST3/Vantage.vst3
```

A `Vantage_Standalone` target is also available for quick auditioning
without a DAW.

### Platform notes

Same as `FMSynths` (see its README for the full rationale): macOS builds a
universal x86_64+arm64 binary by default
(`cmake -B build -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"`), Windows builds
out of the box with any CMake-supported MSVC toolchain, and Linux builds a
working (non-target-platform) `.vst3` for development, needing:

```sh
sudo apt install libasound2-dev libjack-jackd2-dev libx11-dev \
  libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev \
  libxrandr-dev libxrender-dev libfreetype6-dev libfontconfig1-dev \
  libglu1-mesa-dev mesa-common-dev pkg-config
```

## Roadmap

- Per-manufacturer analog modeling for all 3 oscillator and 2 filter slots.
- SH-101-style snappy envelope curve shaping on top of `EnvelopeEngine`.
- User-definable wavetable engine (oscillator model `Wavetable`) with file
  loading.
- The Supermassive-style granular/delay-network reverb algorithm.
- Mono/legato/glide voice-stealing logic and per-voice-vs-global LFO mode.
- 128 factory presets, tagged across Basses/Leads/Pads/Plucks/Keys/Synth
  Drums.
- The custom, decluttered, Surge-XT-inspired `LookAndFeel_V4` UI.
