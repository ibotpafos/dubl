# ДУБЛЬ / DUBL — v1 design

## Decision

Build a native macOS Apple Silicon ARA VST3 plug-in for Studio One. It aligns
one or more vocal doubles to a selected lead-vocal reference. The v1 scope is
same-lyric doubles only; harmony, dialogue replacement, real-time tracking and
a standalone editor are not included.

The product goal is a natural one-click result that preserves the character of
a human double. A conservative result is preferred over an audible artifact.

ДУБЛЬ (technical identifier: `dubl`) is initially developed in public under GPL-3.0. This keeps source
and builds freely available while remaining compatible with JUCE's GPL route.
Any future proprietary distribution requires a separately approved licensing
decision for every dependency; it is not in v1 scope.

## User workflow

1. The producer selects the lead event and one or more double events in Studio
   One, then opens Vocal Align as an ARA Event FX.
2. The plug-in identifies the reference and targets from the selection, shows
   an explicit assignment only if its confidence is insufficient, and analyses
   the events locally.
3. The producer chooses `Natural`, `Tight`, or `Locked`, then presses `Align
   All`.
4. ARA renders the altered double events. Studio One owns project persistence
   and Undo/Restore; the plug-in offers A/B audition and an alignment report.

No audio, features, model inputs, or telemetry leave the computer.

## Controls

The first screen contains only the assignment, preset and three controls:

| Control | Default | Meaning |
| --- | --- | --- |
| Timing tightness | Natural | Amount of local onset and phrase alignment |
| Pitch follow | Medium | Amount the double follows the lead pitch contour |
| Drift preserve | High | Amount of intentional low-frequency pitch movement retained |

`Natural` is the default. `Tight` is for contemporary stacked vocals.
`Locked` permits the most correction but still obeys hard artifact limits.

## Architecture

The plug-in shell is C++ using JUCE, VST3 and ARA SDK. ARA is mandatory for
v1, so the processor analyses whole host events offline instead of relying on
realtime audio buffers.

The independently testable core has five stages:

1. **Analysis:** mono vocal preparation, voicing, pitch/F0, onset, sibilant,
   breath and phrase-boundary estimates.
2. **Pairing:** an ML confidence model and constrained sequence alignment find
   corresponding regions in the lead and double.
3. **Planning:** creates separate smooth time-warp and pitch-warp curves,
   limited by phonetic boundaries, voicing, local signal quality and the user
   controls.
4. **Rendering:** deterministic offline DSP applies the curves with formant
   protection. The initial renderer is behind an interface, with Signalsmith
   Stretch evaluated as a legal MIT baseline rather than coupled to product
   policy.
5. **Validation:** detects instability, excessive deformation, clipped output,
   or low pairing confidence. Unsafe regions are left unchanged and shown in
   the report.

Machine learning is used only in Analysis, Pairing and Validation. It does not
generate audio or replace the deterministic renderer. Models run on-device and
have a versioned manifest for reproducibility.

## Data and languages

Training and evaluation use user-supplied projects only with explicit opt-in.
Raw source material remains local; ingestion produces a separately stored,
anonymized paired dataset plus provenance and consent records. Russian vocals
are the primary v1 dataset. English lead/double pairs are a mandatory
regression set, but no linguistic assumption may be embedded in the core
alignment contract.

## Quality and stability contract

The product must not silently substitute a failed render or modify a target
outside its analysed event. Every render is repeatable from the event content,
settings and model version.

For each benchmark case, record:

- onset and phrase-boundary mismatch before and after;
- pitch-contour distance before and after;
- artifact flags, skipped regions and render failures;
- CPU/RAM/render duration and output peak level;
- blind A/B preference against the original double and commercial references.

Release gates are a deterministic regression suite, corrupted/short/silent
audio cases, repeat open-save-undo-restore in Studio One, and blind listening
on held-out Russian and English sessions. No aggregate metric can override an
audible artifact or data-loss failure.

## Exclusions and follow-up

v1 excludes Windows, Intel macOS, AU/AAX, non-ARA operation, cloud processing,
harmony alignment, lyrics entry and a new DAW. The core renderer and analysis
contracts must remain host-independent so Windows and other formats can be
added after Studio One quality gates are passed.
