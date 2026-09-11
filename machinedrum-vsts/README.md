# Machinedrum VST3 Instruments

Individual VST3 instrument plugins recreating all 30 Elektron Machinedrum
synthesis machines across the **EFM**, **TRX**, and **PI** families, each
shipped as its own standalone instrument plugin. Targets macOS Intel
(x86_64). Built with JUCE via CMake `FetchContent` (no JUCE source is
vendored in this repo).

## Status

All 30 machines are implemented and compile-verified (built and linked as
VST3 bundles on Linux, using the same portable C++/JUCE code that will build
on macOS; the actual macOS/Intel binaries need to be produced on a Mac --
see Building below).

Per-machine scope: everything except the LFO page (dropped per request).
Every instrument has:
- Its **SYN** page, using the real hardware's parameter names/order.
- The shared **TFX** page: AMD/AMF (tremolo), EQF/EQG (1-band parametric
  EQ), FLTF/FLTW/FLTQ (paired lowpass+highpass sharing a base cutoff and
  gap width), SRR (sample-rate reduction).
- The shared **ROUTING** page: DIST, VOL, PAN, DEL, REV (LFOS/LFOD/LFOM
  omitted as LFO-page functionality). DEL/REV feed a small built-in delay
  and reverb per plugin instance, standing in for the hardware's shared
  Rhythm Echo / Gate Box buses, which have no equivalent inside a
  standalone plugin.
- The physical, always-visible **LEV** track-level knob.
- An added **KYBD MODE** toggle: off (default) plays the machine as a
  one-shot drum trigger from any MIDI note; on, MIDI note number pitch-shifts
  the voice around a C3 root, for melodic keyboard playing.

There is intentionally no generic "AMP" page -- the real hardware has none;
each machine's own SYN parameters already shape its amplitude envelope.

### Machine list

| Family | Machines |
|---|---|
| EFM (8) | EFM-BD, EFM-SD, EFM-XT, EFM-CP, EFM-RS, EFM-CB, EFM-HH, EFM-CY |
| TRX (13) | TRX-BD, TRX-B2, TRX-SD, TRX-XT, TRX-CP, TRX-RS, TRX-CB, TRX-CH, TRX-OH, TRX-CY, TRX-MA, TRX-CL, TRX-XC |
| PI (9) | PI-BD, PI-SD, PI-XT, PI-RS, PI-ML, PI-MA, PI-HH, PI-RC, PI-CC |

E12 (12-bit sample-based), ROM/RAM (sample playback), INP/MID (external
I/O), CTR (FX/utility), and GND are out of scope -- either they depend on
Elektron's ROM sample content, or aren't drum synthesis voices.

## Building (on macOS, Intel)

```
cmake -S . -B build -G Xcode
cmake --build build --config Release
```

Requires Xcode command line tools. `CMAKE_OSX_ARCHITECTURES` is forced to
`x86_64` and the deployment target to macOS 10.13 unless overridden.

Built VST3s land in each machine's
`build/machines/<Name>/<Name>_artefacts/Release/VST3/`.

To build a single machine instead of all 30: `cmake --build build --target
EFMBD` (target names are the folder names under `machines/`).

## Architecture

- `shared/` -- headers + sources compiled once per plugin target (not a
  separate static library: JUCE's per-target Linux/macOS dependency wiring
  only applies to targets created via `juce_add_plugin`, so a shared static
  lib wouldn't receive it). Provides:
  - `dsp/` -- oscillators, noise, an FM operator, a state-variable filter,
    AHD envelopes, a wavefolder/soft-clip, and the TFX-page effects
    (tremolo, gap filter, sample-rate reducer, delay).
  - `engines/` -- shared engine implementations reused by machines with
    identical SYN layouts: `TomEngine` (TRX-XT/XC), `HiHatEngine`
    (TRX-CH/OH), `ModalBank` + `CymbalEngine` (the PI family's physically
    modelled resonator machines).
  - `MDLookAndFeel`/`MachineEditor` -- the Machinedrum-faceplate GUI: dark
    chassis, amber LCD-style header, knob rows per page.
  - `MachineProcessor` -- the shared `AudioProcessor`: builds the full
    parameter layout (SYN + TFX + ROUTING + LEV + KYBD) from a machine's
    `MachineInfo`, handles MIDI triggering and keyboard-mode pitch mapping,
    runs the shared TFX/ROUTING signal chain after the machine's own
    `MachineEngine::renderSample()`, and does state save/restore.
- `machines/<Name>/` -- one CMake target per instrument, each producing its
  own VST3. `Source/PluginEntry.cpp` declares the machine's exact SYN
  parameter list and constructs its `MachineProcessor` with the matching
  engine; most machines have their own `Source/<Name>Engine.{h,cpp}`, a few
  share one of the `shared/include/md/engines/` implementations.

## Notes on fidelity

SYN-page parameter names, order, and family/machine list were provided by
the hardware's owner and matched exactly. Beyond that, these are
recreations of each machine's plausible synthesis *approach* (FM operators,
subtractive analog-style synthesis, modal/physically-informed resonators),
not decompiled/bit-exact clones of Elektron's firmware, which isn't public.
Where a parameter's precise internal scaling or an implementation detail
isn't publicly documented, a musically reasonable mapping is used instead
and called out in a comment at the top of the relevant engine header --
most explicitly in `PIBDEngine.h`/`CymbalEngine.h` for the PI family's
HARD/HAMR/AG/AU/BR-style controls.
