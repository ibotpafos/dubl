# ДУБЛЬ / DUBL

Local-first, open-source alignment of a vocal double to a lead vocal: timing
and pitch become tighter while vibrato, formants, breaths and human drift stay
musical.

## Status

**Design and benchmark preparation.** This repository does not yet ship a
plug-in or claim audio quality. The first target is macOS Apple Silicon,
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

## Roadmap

1. Reproducible benchmark corpus and offline analysis prototype.
2. Conservative alignment planner and renderer quality gates.
3. Studio One ARA VST3 integration, A/B, Undo/Restore and regression suite.
4. Public macOS build, then Windows only after the quality gates hold.

## Licence

[GPL-3.0](LICENSE).
