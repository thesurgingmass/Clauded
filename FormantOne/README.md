# Formant One

A VST3 instrument combining an 8-operator FM synthesis engine with a
3-formant "vocal" tone generator, in the spirit of the Yamaha FS1R — the
1998 rack synth that paired DX-style FM with formant/vowel-shaping
synthesis. Built with [JUCE](https://juce.com) and CMake, targeting
Windows, Intel Mac, and Apple Silicon Mac.

> **Not a Yamaha product, and not a bit-exact clone.** This is an
> independent, from-scratch implementation inspired by the *idea* of the
> FS1R — 8 FM operators, an algorithm-based routing scheme, three formant
> generators, a resonant filter, and an LFO. It doesn't use any Yamaha
> code, ROM data, or assets, and it is **not a reverse-engineered or
> "sample accurate" copy**: Yamaha never published the FS1R's internal
> algorithm-routing diagrams, its envelope generator's exact rate/time
> curve, or the circuit design of its formant generator, and there was no
> hardware or reference audio available while building this to capture
> and match against. Where the real internals are undocumented, this
> plugin implements a faithful reconstruction of *what that part of the
> architecture is for* (see "How this maps to the FS1R" below), built from
> general FM- and formant-synthesis principles rather than a copy of
> Yamaha's own implementation. "Yamaha" and "FS1R" are trademarks of
> Yamaha Corporation; no affiliation is implied, which is also why this
> plugin ships as "Formant One" rather than under Yamaha's product name.
>
> If you have real FS1R patches, audio captures, or hardware to compare
> against, matching this engine's output more closely against them is a
> natural next step — just not one this pass could do blind.

## What's implemented

### FM engine

- **8 operators**, each with:
  - a ratio (coarse multiplier + fine tune) against the note's pitch, *or*
    a fixed Hz frequency (both modes exist on real FM hardware, e.g. for
    metallic/bell partials that shouldn't track the keyboard)
  - level (also acts as modulation index when the operator is a modulator)
  - a 4-stage rate/level envelope (see "Envelopes" below) — operator 1 also
    has self-feedback
- **24 selectable algorithms**: the 22 possible ways to partition 8
  operators into independent serial modulation "stacks" (spanning fully
  serial — deep, clangorous FM — through fully parallel/additive —
  organ/bell-like), generated programmatically (see
  `Source/FormantOne/FMAlgorithms.cpp`), plus 2 hand-built algorithms with
  cross-modulation for variety.

### Formant ("vocal") engine

- **3 formant generators** (the classic F1/F2/F3 vocal-formant idea), each
  a source/filter pair with its own frequency, bandwidth, level, and
  4-stage envelope:
  - **Voiced** excitation: PAF-style ("phase-aligned formant") synthesis —
    a windowed sine burst at the formant frequency, retriggered once per
    fundamental-pitch period.
  - **Unvoiced** excitation: a shared per-voice noise source run through a
    resonant bandpass tuned to the same formant frequency/bandwidth.
  - A **Voiced/Unvoiced** balance crossfades between the two.
- An **FM/Formant balance** control mixes the FM engine's output against
  the formant engine's output before the shared filter/amp/drive chain.

### Shared per-voice chain

- A multimode resonant filter — **LPF24** (24dB/oct, two cascaded 12dB
  stages), **LPF12**, **HPF12**, **BPF**, **BEF** (band-eliminate/notch) —
  with its own 4-stage envelope and key-tracking.
- An output-stage **amp envelope** (4-stage) and a tanh **drive/saturation**
  stage.
- One per-voice **LFO** (sine/triangle/saw/square/sample-and-hold) with
  rate, delay/fade-in, and independent depth sends to pitch, amplitude, and
  filter cutoff.
- 8-voice polyphony, full MIDI (note on/off, velocity, pitch bend).

### Envelopes

Every operator, formant, the amp stage, and the filter use the same
4-stage rate/level envelope shape found on classic Yamaha FM hardware
(`Source/Common/RateLevelEnvelope.*`) rather than a simple ADSR: three
time+level segments run while a note is held (attack → level 1 → decay 1 →
level 2 → decay 2 → sustain), and a note-off doesn't wait for the sustain
level to interrupt — it jumps straight from *wherever the envelope
currently is* into the release segment. That interrupt-from-anywhere
behavior is the well-documented, distinctive part of these envelopes; the
segments themselves are simple linear ramps (see the disclaimer above on
why this isn't an attempt at the hardware's exact 0-99 rate curve).

## How this maps to the FS1R

| FS1R concept | This plugin |
| --- | --- |
| 8 FM operators, algorithm-based routing | 8 operators, 24 algorithms (see above — a systematic reconstruction, not the FS1R's own list) |
| 3 formant generators, voiced/unvoiced | 3 formant generators, source/filter model, voiced/unvoiced balance |
| Operator/pitch/filter EG (rate/level) | `RateLevelEnvelope`, same 4-stage shape and note-off behavior |
| Filter section (LPF24/LPF12/HPF12/BPF/BEF) | Same 5 types, one shared multimode filter |
| Pitch/amp/filter LFO | One per-voice LFO, 3 depth sends |
| 4-part multitimbral "Performance" mode | **Not implemented** — this is a single-voice instrument plugin |
| Formant Sequence (programmable vowel morphing over steps) | **Not implemented** — a formant's shape can still evolve via its own envelope |
| Audio-input vocoder mode | **Not implemented** — that's a hardware mic/line-input feature, out of scope for a plugin instrument |
| System/insert effects (reverb, chorus, distortion racks) | **Not implemented** beyond the per-voice drive stage |

## Building

Requires CMake 3.22+ and a C++17 compiler. JUCE itself is fetched
automatically by CMake (`FetchContent`, pinned to release 8.0.15) — no
manual JUCE install needed.

```sh
cd FormantOne
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target FormantOne_VST3 --config Release --parallel
```

The resulting plugin lands in:

```
build/Source/FormantOne/FormantOne_artefacts/Release/VST3/Formant One.vst3
```

There's also a **Standalone** app target (`FormantOne_Standalone`), a plain
double-clickable app with the same synth engine and GUI, no DAW/VST3
scanning involved. It's the fastest way to sanity-check that the plugin
itself works before troubleshooting a host's plugin scan:

```sh
cmake --build build --target FormantOne_Standalone --config Release --parallel
open "build/Source/FormantOne/FormantOne_artefacts/Release/Standalone/Formant One.app"   # macOS
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

`.github/workflows/formant-one-build.yml` builds the plugin on
`windows-latest` and `macos-latest` (universal binary, verified with
`lipo -info`) on every push/PR touching `FormantOne/`, and uploads the
built `.vst3` bundle as a workflow artifact.

### Known risk (built, not DAW-tested)

This was written and reviewed, and does build cleanly as a Linux VST3 in
the sandbox it was developed in (see "Linux" above) — but it has **not**
been loaded into a real DAW or heard, because that requires a real
Windows/macOS host. The GitHub Actions workflow is what actually produces
verified macOS/Windows binaries; loading the plugin in a DAW on your Mac
is the first true end-to-end test. If something doesn't sound right or a
control misbehaves, that's the most likely place a bug would surface.

## Architecture

```
FormantOne/
  CMakeLists.txt          Top-level: fetches JUCE, sets macOS universal-binary
                            defaults, adds the two subdirectories below.
  Source/
    Common/                 FSCore static library — DSP shared by the plugin
      RateLevelEnvelope.*     4-stage rate/level EG w/ note-off-interrupts-anywhere
      FMOperator.*            phase-modulated sine operator (ratio/fixed, feedback, EG)
      FormantGenerator.*      PAF voiced grain + resonant-noise unvoiced source/filter
      NoiseSource.h            shared per-voice white noise source
      MultiModeFilter.*        TPT state-variable filter: LPF24/12, HPF12, BPF, BEF
      Lfo.*                    one modulation LFO w/ delay/fade-in
      Drive.*                  tanh saturation stage
      DspUtils.h               note/frequency/ratio math helpers
    FormantOne/              The plugin
      FMAlgorithms.*           the 24-algorithm routing table (partition-generated)
      FormantOneVoice.*        per-voice rendering (FM + formant -> filter -> amp -> drive)
      FormantOneSound.h
      Parameters.*             APVTS parameter layout + per-block parameter readout
      PluginProcessor.*
      PluginEditor.*           grouped-controls GUI, generic ParamSlider/Combo/Toggle
```
