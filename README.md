# OctaClone

A standalone macOS app, native to Apple Silicon, inspired by the Elektron
Octatrack: an 8-track sample sequencer with parameter locks, polymetric
per-track step lengths, scene morphing, and full MIDI control.

It's pure Swift + Apple frameworks (SwiftUI, AVFoundation, CoreMIDI) — no
third-party dependencies, so it builds with just the Xcode Command Line
Tools.

> **Note on how this was built:** I didn't have network access to the
> Octatrack manual while writing this (the request linked
> elektron.se, which this sandbox's network policy blocks), so the feature
> set below is built from general knowledge of the Octatrack's design
> rather than a line-by-line reading of the manual. It's also been written
> and reviewed, but **not compiled** — this dev environment is Linux, and
> Swift on Linux doesn't have SwiftUI/AVFoundation/CoreMIDI. The first
> build on an actual Mac may turn up a typo or two; see "Known risk" below.

## Features

- **8 audio tracks**, each a RAM-loaded sample player (Octatrack calls this
  a "flex machine" — there's no streamed "static machine" here, see
  Roadmap).
- **Parameter locks ("p-locks")**: any step can override volume, pan,
  pitch, sample start/length, filter cutoff/resonance, or delay
  send/time/feedback for just that trig.
- **Lock trigs**: a step can apply parameter locks *without* retriggering
  the sample, same as the Octatrack.
- **Polymetric sequencing**: each track has its own step count (1–64) and
  its own scale (2x, 1x, 1/2x, 1/4x), so tracks can run independent-length,
  independent-speed loops against each other.
- **16 patterns** with pattern-chaining queued to the next bar while
  playing.
- **Scenes + crossfader**: snapshot a track's parameters into Scene A/B and
  morph between them live with a crossfader.
- **Per-track effects**: a filter (EQ) and a delay per track, all
  parameter-lockable.
- **Full MIDI control**:
  - Each track has its own MIDI channel + note that triggers it directly
    (drum-pad style).
  - MIDI CC → any parameter, with a MIDI-learn workflow.
  - MIDI Program Change selects a pattern.
  - MIDI clock in (syncs tempo and start/stop/continue to external gear)
    or MIDI clock out (drives external gear from OctaClone's clock).
  - Each track can optionally also send a MIDI note out when it triggers,
    to sequence outboard gear.
- **Waveform view** with draggable sample start/end trim handles.
- **Save/load projects** as JSON, with sample files tracked by bookmark so
  moving/renaming samples on the same volume doesn't break a project.

## Requirements

- An Apple Silicon Mac (M1 or later) running macOS 13 Ventura or later.
- Xcode 15+ **or** just the Xcode Command Line Tools
  (`xcode-select --install`) — there are no external package dependencies.

## Building & running

**Quick way (Xcode):**

1. Open `Package.swift` in Xcode (File → Open, pick the package folder).
2. Select the `OctaClone` scheme, destination "My Mac".
3. Run (⌘R).

**Command line, as a distributable `.app`:**

```sh
./Scripts/build_app.sh
open OctaClone.app
```

This does `swift build -c release --arch arm64` and wraps the resulting
binary into `OctaClone.app` with an ad-hoc code signature (so Gatekeeper
doesn't block a local unsigned build). If you copy the app somewhere else
and macOS still refuses to open it, clear the quarantine flag:

```sh
xattr -dr com.apple.quarantine OctaClone.app
```

**Command line, just to run it:**

```sh
swift run -c release
```

## Using it

- **Load a sample**: click "Load Sample…" on a track strip.
- **Sequence it**: click steps in the grid to add/remove trigs. Click a
  step to select it — the panel below the grid shows that step's trig type
  (Note/Lock), velocity, micro-timing, and parameter locks.
- **Trim a sample**: drag the yellow handles on the waveform view.
- **Scenes**: adjust a track's base parameters, then "Store T# → A" (or
  B); move the crossfader to morph live between the two stored states.
- **MIDI**: open the MIDI panel (toolbar button) to pick your input/output
  device, choose the clock source, and set up CC mappings via "Learn".
  Each track's own MIDI channel/note (for direct note-triggering) is set
  on its track strip.

## Architecture

```
Sources/OctaClone/
  App/          Composition root (AppState) and the SwiftUI App entry point
  Models/       Codable value types: Track, Pattern, Step, Project, MIDI mapping
  Audio/        AVAudioEngine graph: per-track player → pitch → filter → delay → mixer
  Sequencer/    Look-ahead scheduler; drives audio from either an internal
                clock or incoming MIDI clock ticks
  MIDI/         CoreMIDI wrapper: I/O, message parsing, MIDI learn
  Persistence/  Project save/load as JSON + sample bookmarks
  UI/           SwiftUI views
```

The sequencer uses the same look-ahead pattern most audio-timing code uses
(schedule slightly into the future using `AVAudioTime` host times, wake up
frequently to keep the schedule topped up) rather than driving playback
directly off a `Timer`, so trig timing doesn't drift or jitter with UI
thread hiccups. Ticks run at 24 PPQN (matching MIDI clock resolution), and
each track independently derives its own step boundaries from that shared
tick counter — that's what makes the per-track polymetric scale/step-count
feature possible.

## MIDI reference

| Message | Effect |
|---|---|
| Note On (channel = track's MIDI channel, note = track's base note) | Triggers that track immediately |
| Note Off (same, for a looping track) | Stops that track |
| Control Change, per a learned mapping | Sets the mapped parameter live |
| Program Change | Selects pattern `program mod 16` (toggle in MIDI settings) |
| Clock (0xF8) | Advances the sequencer when Clock Source = External |
| Start / Continue / Stop | Starts/resumes/stops playback when synced externally |

## Known risk (unverified build)

This was written without a macOS/Xcode toolchain available to compile
against, so treat the first build as the real first compile. The riskiest
code, if anything breaks, is likely:

- `MIDIManager.handle(packetList:)` — the manual pointer walk over a
  `MIDIPacketList` is the classic place CoreMIDI wrapper code gets subtly
  wrong; I've written it to keep the pointer inside CoreMIDI's own buffer
  (rather than a local copy) so `MIDIPacketNext` stays valid across
  multiple packets, but I couldn't run it against a real device.
- `SampleAsset`'s `AVAudioConverter` path (only exercised when a loaded
  file's format doesn't already match the engine's 44.1kHz stereo format).

If either misbehaves, both are self-contained and easy to swap out.

## Roadmap / deliberate simplifications

This covers the core of what makes the Octatrack the Octatrack, but it's
not a 1:1 clone. Not implemented (contributions/requests welcome):

- **Static machines** (streamed-from-disk playback) — everything here is
  RAM-loaded ("flex" only). Fine for drum hits and short loops; long
  sample sets will just use more memory.
- **Audio input / Thru machines / resampling** — no live audio input or
  recording into a track.
- **Neighbor machines**, pickup machines, and the second effects slot per
  track (currently one filter + one delay, not the Octatrack's fuller
  effects list — no compressor, reverb, EQ-as-separate-slot, etc.).
- **Dedicated MIDI tracks** — instead, any audio track can optionally also
  emit a MIDI note when it triggers. The real Octatrack has 8 separate
  MIDI-only tracks with their own arpeggiator/LFOs; that's a bigger
  feature than this covers.
- **Arranger / song mode** — pattern chaining is manual (queue next
  pattern) rather than a programmed arrangement.
- **Per-project effects/LFO assignment UI beyond parameter locks.**
