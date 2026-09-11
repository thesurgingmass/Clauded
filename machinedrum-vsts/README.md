# Machinedrum VST3 Instruments (WIP)

Individual VST3 instrument plugins recreating Elektron Machinedrum drum
synthesis machines, targeting macOS Intel (x86_64). Built with JUCE via
CMake `FetchContent` (no JUCE source is vendored in this repo).

## Status

This is an in-progress build. Current scope (per hardware owner confirmation):
**EFM**, **TRX**, and **PI** machine families (all pure-synthesis, no ROM
sample dependency) -- LFO page intentionally omitted per request, everything
else (SYN, FLT, AMP) included, plus an added **KYBD MODE** toggle per
instrument that switches the voice from one-shot drum-trigger to keyboard
pitch-tracking.

- [x] Shared architecture: DSP building blocks, Machinedrum-style faceplate
      UI (`MDLookAndFeel`/`MachineEditor`), generic `MachineProcessor` with
      MIDI trigger + keyboard-mode logic, per-machine state save/restore.
- [ ] SYN-page parameter names/ranges are being corrected to match the real
      hardware exactly (an earlier pass mistakenly used Analog Rytm-style
      naming -- since fixed).
- [ ] FLT/AMP shared page parameters: pending exact hardware names.
- [ ] Machine engines: in progress.

## Building (on macOS, Intel)

```
cmake -S . -B build -G Xcode
cmake --build build --config Release
```

Requires Xcode command line tools. `CMAKE_OSX_ARCHITECTURES` is forced to
`x86_64` and the deployment target to macOS 10.13 unless overridden.

Built VST3s land in each machine's `build/<Machine>_artefacts/Release/VST3/`.

## Architecture

- `shared/` -- `MDShared` static library: DSP primitives (`dsp/`), the
  Machinedrum-faceplate look-and-feel and generic editor, and
  `MachineProcessor`, the shared `AudioProcessor` every machine plugin uses.
  A machine only supplies a `MachineInfo` (its SYN knob layout) and a
  `MachineEngine` (its SYN-stage synthesis) -- the FLT/AMP signal chain,
  MIDI handling, and keyboard-mode logic are shared.
- `machines/<Name>/` -- one CMake target per instrument, each producing its
  own VST3. `Source/PluginEntry.cpp` declares the machine's SYN params and
  wires up its engine; `Source/<Name>Engine.{h,cpp}` implements the
  synthesis.

## Notes on fidelity

These are recreations of each machine's documented synthesis *approach*,
not decompiled/bit-exact clones of Elektron's firmware -- that source isn't
public. SYN-page parameter names and ranges are being matched to the real
hardware; where a DSP implementation detail isn't publicly documented, a
reasonable period-appropriate technique is used instead and called out in
code comments.
