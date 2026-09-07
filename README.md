# ДУБЛЬ / DUBL

Local-first, open-source alignment of a vocal double to a lead vocal: timing
and pitch become tighter while vibrato, formants, breaths and human drift stay
musical.

## Status

**Working offline timing baseline.** The repository builds a command-line
product that analyses a mono lead/double pair and safely writes a timing- and
globally pitch-aligned WAV plus a JSON report. It does not yet ship the Studio One plug-in or claim
production vocal quality. The first plug-in target is macOS Apple Silicon,
Studio One, and ARA VST3. v1 handles doubles with the same lyric as the lead;
it deliberately does not attempt harmony alignment.

## Intended workflow

1. Select a lead event and its vocal doubles in Studio One.
2. Open ДУБЛЬ as an ARA Event FX.
3. Choose `Natural`, `Tight`, or `Locked`, then press `Align All`.
4. Audition A/B; Studio One retains ownership of persistence and Undo/Restore.

The design prefers leaving an uncertain region untouched over creating an
audible artifact.

## Technology direction

- C++ plug-in core using JUCE, VST3 and the ARA SDK.
- On-device ML for vocal structure, correspondence confidence and artifact
  detection — never cloud audio processing or generated audio.
- Deterministic offline DSP rendering behind a replaceable renderer interface.
- Russian voice material is the primary dataset; English pairs remain a
  mandatory regression set from day one.

## How to follow or contribute

Read the [v1 design](docs/superpowers/specs/2026-09-07-vocal-align-studio-one-design.md)
and [contribution guide](CONTRIBUTING.md). The project is GPL-3.0, including
the initial public build path. Do not add actual vocal material without written
consent and documented provenance.

## Local core and offline renderer

The analyser writes a versioned, path-safe JSON warp plan. The renderer applies
the conservative timing map, a confidence-gated pitch correction with formant
compensation, and publishes a new 16-bit PCM mono WAV atomically.
Existing files and either input WAV are never overwritten.

Requirements: macOS on Apple Silicon, AppleClang with C++20, and CMake 3.25 or
newer.

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Run the analyser with 16-bit PCM or 32-bit float mono WAV files at 44.1 or
48 kHz:

```bash
./build/dubl_analyse \
  --lead /path/to/lead.wav \
  --double /path/to/double.wav \
  --report /path/to/report.json
```

The report contains sample rates, frame counts, confidence, identity/safe point
counts and the bounded warp map. It never contains input paths or waveform
samples.

Render a new aligned WAV and a compact machine-readable report:

```bash
./build/dubl_render \
  --lead /path/to/lead.wav \
  --double /path/to/double.wav \
  --output /path/to/aligned.wav \
  --report /path/to/render-report.json \
  --mode natural
```

Modes are `natural` (default), `tight`, and `locked`. Pitch is currently one
bounded global correction derived from confident aligned voiced frames; uncertain
material remains unchanged. Time-varying note correction, learned artifact
detection, the trained on-device model and ARA/VST3 integration remain roadmap
work. The JSON report records whether pitch was actually applied, the factor,
mode, and evidence count.

## Roadmap

## Native standalone preview

Run `bash standalone/build-app.sh` on macOS, then open
`build-standalone/DUBL.app`. The application bundles the offline engine and
requires no DAW. Drop two or more mono WAV tracks into one DAW-style timeline,
select the tracks to process, choose a mode and press Align All. The processed
waveforms replace the originals in place; Before/After switches the whole
session for instant comparison. Results save as a complete batch to a new
folder, and existing destinations are not overwritten.

This preview supports WAV drag-and-drop, individual playback and synchronized
multitrack audition with a conservative mix gain. The first selected track is
currently the internal alignment reference; there is no separate lead/output
lane in the product UI. Processing remains the experimental timing/global-pitch
baseline described above.

1. Reproducible benchmark corpus and offline analysis prototype.
2. Conservative alignment planner and renderer quality gates.
3. Studio One ARA VST3 integration, A/B, Undo/Restore and regression suite.
4. Public macOS build, then Windows only after the quality gates hold.

## Experimental macOS VST3 shell

Build the Apple Silicon plug-in shell explicitly:

```bash
cmake -S . -B build-plugin \
  -DDUBL_BUILD_PLUGIN=ON \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build-plugin --target DUBL_VST3 --parallel
ctest --test-dir build-plugin -R dubl_test_plugin --output-on-failure
```

The bundle is written to
`build-plugin/DUBL_artefacts/Release/VST3/DUBL.vst3`. Copy it to
`~/Library/Audio/Plug-Ins/VST3/` and rescan plug-ins in Studio One.

This milestone is intentionally an audio-safe pass-through shell: its mode
parameter persists and its one-button interface loads, but the Align button
does not yet receive or replace Studio One event audio. That requires the next
ARA document-controller milestone; use `dubl_render` for actual alignment now.

## Licence

[GPL-3.0](LICENSE).
See [third-party notices](THIRD_PARTY_NOTICES.md) for MIT-licensed DSP dependencies.
