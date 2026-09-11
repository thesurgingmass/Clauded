# FM Synths

Two VST3 instruments built around FM synthesis engines in the spirit of the
Elektron Digitone II: **FM Tone** (a 4-operator melodic FM synth) and
**FM Drum** (an FM-based percussion synth: FM body + noise + transient).
Built with [JUCE](https://juce.com) and CMake, targeting Windows, Intel
Mac, and Apple Silicon Mac.

> **Not an Elektron product.** This is an independent, from-scratch
> implementation inspired by the *idea* of Digitone II's FM TONE and FM
> DRUM machines — operators, algorithms, envelopes, a multimode filter,
> drive. It is not a licensed product, doesn't use any Elektron code or
> assets, and isn't a bit-exact clone: Elektron hasn't published exact
> internal algorithm-routing diagrams or DSP source, so the operator
> routing and envelope shapes here are a faithful-in-spirit
> reconstruction from general FM-synthesis principles and the publicly
> documented *behavior* of the hardware, not a reverse-engineered copy.
> "Digitone" and "Elektron" are trademarks of Elektron Music Machines MAV
> AB; no affiliation is implied.

## What's implemented

Per the brief, this covers the **core synthesis engine** — operators,
algorithms, envelopes, filter, drive — and deliberately **excludes LFOs
and the send effects** (delay/reverb/chorus) that sit on the hardware's
FX page. See "Deliberate simplifications" below for the full list.

### FM Tone

- 4 operators (**C, A, B, D**, matching the hardware's naming), each with:
  - a coarse ratio (16 common FM ratios, 0.25–16×) + fine tune (±1 semitone)
  - level (also acts as modulation index when the operator is a modulator)
  - its own ADSR envelope (attack / decay / "end level" / release)
  - self-feedback, on operator C only
- **8 selectable algorithms**, from a fully serial chain (`D>B>A>C`) to
  fully parallel/additive (`C+A+B+D`) — see `Source/FMTone/FMAlgorithms.h`
  for the exact routing of each, and the note above on why these are an
  approximation rather than a reverse-engineered copy.
- A shared amp envelope (ADSR) shaping the algorithm's carrier(s).
- A resonant multimode filter (low/high/band-pass) with its own envelope
  and key-tracking.
- A drive/saturation stage (tanh soft-clipping with level-compensated
  makeup gain).
- 16-voice polyphony, full MIDI (note on/off, velocity, pitch bend).

### FM Drum

- **Body**: a 2-operator FM stack (op2 modulates op1) with ratio, tune,
  modulation index, and self-feedback — the tonal "thump" of a kick/tom/etc.
- **Pitch sweep**: an exponential pitch envelope (amount + decay time)
  layered on top of the body's pitch, for the classic FM-drum "pop".
- **Noise**: white noise through its own low/high/band-pass tone filter
  and ADSR envelope.
- **Transient**: a very short noise burst (its own level + decay) for
  click/attack transient.
- The same shared amp envelope, multimode filter (+ envelope + key-track),
  and drive stage as FM Tone.
- Monophonic: a single voice, so a new hit immediately cuts off and
  retriggers whatever's still sounding — no layering/overlap on the same
  drum, matching how a drum machine's voice behaves.

### Shared DSP core (`Source/Common/`)

Both plugins are built on the same small DSP library (`FMCore`):
`FMOperator` (a phase-modulated sine operator with its own envelope and
optional self-feedback), `MultiModeFilter` (state-variable filter +
envelope + key-track), and `Drive` (tanh saturation).

## Deliberate simplifications

- **No LFOs.** The hardware's 3 per-track LFOs (routable to almost any
  parameter) aren't implemented — out of scope per the brief.
- **No send effects.** Delay, reverb, chorus, and the overdrive/compressor
  on the FX page aren't implemented — only the per-voice drive stage that's
  part of the synth voice itself. Out of scope per the brief.
- **No arpeggiator, sequencer, or sample-based content** — these are pure
  MIDI-in instruments, play them from a DAW's piano roll or a keyboard.
- **Operator envelopes are full ADSR**, not the hardware's more nuanced
  per-stage curves/behaviors — attack/decay/"end level"(=sustain)/release
  covers the same shape without matching every curve detail.
- **Fixed voice counts**: FM Tone is 16-voice polyphonic, FM Drum is
  monophonic (1 voice); neither is currently user-configurable.
- **GUI is functional, not skeuomorphic** — grouped rotary knobs and combo
  boxes bound to every parameter (scrollable, resizable), not a graphical
  recreation of the hardware's screen/encoder layout. All parameters are
  also exposed as regular DAW-automatable plugin parameters, so a DAW's
  own generic editor works too.

## Building

Requires CMake 3.22+ and a C++17 compiler. JUCE itself is fetched
automatically by CMake (`FetchContent`, pinned to release 8.0.15) — no
manual JUCE install needed.

```sh
cd FMSynths
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target FMTone_VST3 --config Release --parallel
cmake --build build --target FMDrum_VST3 --config Release --parallel
```

Resulting plugins land in:

```
build/Source/FMTone/FMTone_artefacts/Release/VST3/FM Tone.vst3
build/Source/FMDrum/FMDrum_artefacts/Release/VST3/FM Drum.vst3
```

### Platform notes

- **Windows**: builds out of the box with Visual Studio 2022 (or any
  CMake-supported MSVC toolchain). No extra flags needed.
- **macOS (universal Intel + Apple Silicon)**: pass the architecture list
  explicitly at configure time —
  `cmake -B build -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"` (the top-level
  `CMakeLists.txt` also defaults to this automatically on macOS if you
  don't set it). Verify with `lipo -info` on the built binary inside the
  `.vst3` bundle.
- **Linux**: builds and produces a working `.vst3` too (JUCE supports
  Linux VST3), though it isn't one of the three target platforms. This is
  how the DSP/plugin code was actually compiled and validated in the
  environment this was built in — see needed system packages below.
  ```sh
  sudo apt install libasound2-dev libjack-jackd2-dev libx11-dev \
    libxcomposite-dev libxcursor-dev libxext-dev libxinerama-dev \
    libxrandr-dev libxrender-dev libfreetype6-dev libfontconfig1-dev \
    libglu1-mesa-dev mesa-common-dev pkg-config
  ```

### CI

`.github/workflows/fm-synths-build.yml` builds both plugins on
`windows-latest` and `macos-latest` (universal binary, verified with
`lipo -info`) on every push/PR touching `FMSynths/`, and uploads the
built `.vst3` bundles as workflow artifacts.

## Architecture

```
FMSynths/
  CMakeLists.txt         Top-level: fetches JUCE, sets macOS universal-binary
                          defaults, adds the three subdirectories below.
  Source/
    Common/               FMCore static library — DSP shared by both plugins
      FMOperator.*         phase-modulated sine operator (ratio, feedback, ADSR)
      MultiModeFilter.*     state-variable filter + envelope + key-track
      Drive.*               tanh saturation stage
      DspUtils.h            note/frequency/ratio math helpers
    FMTone/                4-operator FM synth plugin
      FMAlgorithms.h         the 8 operator-routing tables
      FMToneVoice.*          per-voice rendering (operators -> algorithm -> amp env -> filter -> drive)
      Parameters.*            APVTS parameter layout + per-block parameter readout
      PluginProcessor.*       juce::AudioProcessor: synth setup, MIDI, state save/load
      PluginEditor.*          generic knob/combo-box GUI bound to every parameter
    FMDrum/                similarly: body/pitch/noise/transient -> amp env -> filter -> drive
```

Each operator/voice renders directly against a per-block-refreshed
`VoiceParameters` struct read out of the plugin's
`AudioProcessorValueTreeState`, so every parameter is fully
host-automatable and saved/restored with plugin state.

## Known risks / what to check first if something misbehaves

This was built and validated by compiling to a working `.vst3` on Linux
(the DSP and plugin code is identical across platforms — JUCE abstracts
the platform-specific bits), but it has **not been run in an actual DAW
or checked with `pluginval`** in this environment, and the **macOS
universal-binary and Windows builds specifically have not been run
through CI yet** (the workflow is in place but hasn't executed). Before
relying on it:

1. Run the GitHub Actions workflow (or build locally) on Windows and
   macOS and confirm both plugins load in a host.
2. Load each plugin in a DAW, play some notes, and sweep a few
   parameters (especially `Algorithm` on FM Tone, and `Body Ratio`/`Body
   Index` on FM Drum) to confirm audible, non-clipping output across the
   parameter ranges.
3. Run [`pluginval`](https://github.com/Tracktion/pluginval) against both
   `.vst3` bundles for standard host-compatibility checks (parameter
   automation, state save/load, threading).

## Roadmap / not implemented

- LFOs (per the brief's scope for this pass).
- Send effects: delay, reverb, chorus (per the brief's scope).
- A skinned/graphical UI matching the hardware's look.
- Preset browser / factory preset bank.
- Mono/legato modes, glide/portamento.
- AU and CLAP formats (VST3 only for now — JUCE's `juce_add_plugin`
  makes adding `FORMATS AU` on macOS or `FORMATS CLAP` a small follow-up
  if wanted, since the underlying `AudioProcessor` doesn't change).
