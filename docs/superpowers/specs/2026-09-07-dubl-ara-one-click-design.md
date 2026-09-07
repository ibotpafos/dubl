# DUBL: one-click ARA vocal alignment

## Product contract

DUBL is an ARA Event FX workflow for aligning two or more selected vocal events
inside a DAW. The user selects the events and applies DUBL. Processing starts
automatically; there is no separate capture step, fixed Guide/Dub/Output lane,
or required Align button in the plug-in editor.

The selected tracks are peers. DUBL derives a robust shared timing and pitch
target from the group instead of asking the user to nominate a permanent lead.
The original events remain recoverable at all times.

The first supported host is Studio One on Apple Silicon macOS. The processing
core remains host-independent so Windows and other ARA hosts can follow without
rewriting the alignment engine.

## Host interaction

1. The user selects at least two compatible vocal events in Studio One.
2. The user applies DUBL as an ARA/Event FX extension.
3. Studio One creates the ARA document objects, audio sources, modifications,
   playback regions, and plug-in renderer roles.
4. DUBL snapshots the assigned regions and begins analysis in the background.
5. The DAW continues playing the original audio until a complete group result
   is available.
6. DUBL atomically publishes the completed group to its playback renderer.
7. Its editor displays the selected tracks in one compact arrangement view and
   redraws each processed waveform in place.
8. A single Before/After control switches the entire group for audition.

ARA 2.3 does not guarantee that a host will display the plug-in's rendered
waveform envelope in its native arrangement. DUBL therefore owns the processed
waveform display in its ARA editor. Studio One's arrangement may continue to
show the source waveform while playback uses the processed result. Committing
or replacing host events is outside the one-click v1 workflow.

## Editor experience

The editor contains one DAW-style timeline, not separate Guide, Dub, and Output
sections. Every assigned event appears as an equal track row with a colored
audio clip, source name, waveform, selection state, and analysis status.

The default interface exposes only:

- group progress and readiness;
- Before and After audition;
- Natural, Tight, and Locked modes;
- low-confidence markers that explain where DUBL deliberately corrected less;
- a concise error or compatibility message when processing cannot proceed.

Natural is the default. Detailed algorithm controls stay out of the primary
flow until listening evidence proves they are needed. Unsupported controls must
not be displayed as inactive or decorative settings.

## Group analysis

Processing begins from an immutable snapshot of the assigned playback regions.
The analysis pipeline performs:

1. input validation and channel/sample-rate normalization;
2. vocal activity, voiced/unvoiced, transient, onset, breath, and pitch feature
   extraction;
3. phrase segmentation using overlap and compatible vocal activity;
4. pairwise confidence-aware alignment between the events in each phrase;
5. outlier rejection for missed words, ad-libs, noise, and unreliable pitch;
6. construction of a robust shared timing and pitch target;
7. generation of a bounded correction plan for each event.

The shared target is a latent consensus. Per phrase, it uses robust median
landmarks and pairwise distances rather than blindly averaging sample data. A
medoid performance may supply a local landmark when the consensus is ambiguous,
but it does not become a permanent user-visible lead.

The first production implementation may stage this capability: start with a
deterministic confidence-aware medoid per phrase, then replace the target
builder with a learned consensus model without changing the ARA or renderer
interfaces.

## AI and DSP boundary

Machine learning is local and advisory. It may identify phoneme boundaries,
vocal activity, breaths, note regions, outliers, and safe correction strength.
It does not generate replacement vocals in v1.

Rendering is deterministic DSP. It applies the approved time and pitch plan
with formant preservation, bounded local stretch ratios, smooth transitions,
and protection for consonants, breaths, unvoiced material, and low-confidence
regions. Identical inputs, model version, mode, and settings must produce the
same result.

The product must run without a cloud connection. Models and algorithm versions
are recorded in the ARA archive so restored projects remain explainable.

## Concurrency and lifecycle

The ARA model graph and editor state remain on their required host/UI isolation
domains. Audio extraction, feature analysis, plan construction, and offline
rendering run outside the audio callback and UI thread.

Each analysis request receives a monotonically increasing generation ID and an
immutable region snapshot. Editing, moving, trimming, replacing, or removing a
source region invalidates the affected generation and schedules a new one.
Cancelled or stale work may finish internally but cannot publish a result.

The renderer reads only an immutable, fully prepared render snapshot. Publishing
the group result is atomic: playback uses either the complete original group or
the complete processed group, never a mixture created by partial completion.
No allocation, file I/O, model inference, blocking lock, or analysis occurs in
the real-time audio callback.

## Persistence and Undo

The ARA document archive stores stable source/modification identifiers,
algorithm and model versions, modes, correction-plan metadata, confidence data,
and sufficient cache references to reproduce or safely rebuild the result.

Before/After is non-destructive and does not modify source media. Host edits and
DUBL state changes participate in the ARA editing lifecycle. A single host Undo
must return to the preceding audible state. Closing and reopening a project must
restore the same audible result or deterministically rebuild it before enabling
After playback.

Temporary render files use new, scoped directories and atomic publication.
Failed and cancelled generations remove only their own unpublished artifacts.
Source files and pre-existing destinations are never overwritten.

## Failure behavior

DUBL refuses processing and keeps original playback when:

- fewer than two usable regions are assigned;
- required audio access fails;
- channel layout, sample rate, or region metadata cannot be normalized safely;
- the group has no compatible overlapping vocal phrases;
- the renderer cannot publish a complete result;
- archive restoration is incompatible and a deterministic rebuild cannot run.

Low confidence is not automatically an error. DUBL reduces correction locally,
marks the affected range, and preserves the source when that is safer. Errors
identify the affected track or phrase and provide one clear recovery action.

## Component boundaries

- `AraDocumentAdapter`: converts ARA sources, modifications, and playback
  regions into immutable engine snapshots and emits invalidation events.
- `GroupCoordinator`: groups regions, owns generation IDs, cancellation, state
  transitions, and atomic publication.
- `ConsensusAnalyzer`: extracts features, calculates pairwise evidence, rejects
  outliers, and produces the shared phrase target with confidence.
- `CorrectionPlanner`: converts the target into bounded per-event timing and
  pitch plans.
- `OfflineRenderer`: produces immutable processed buffers/cache files using the
  existing deterministic DSP core.
- `AraPlaybackRenderer`: renders published snapshots during DAW playback without
  blocking or allocating in the audio callback.
- `AraArchive`: persists versioned document state and restores or invalidates
  cached results safely.
- `DublEditor`: shows the unified timeline, progress, confidence markers,
  Before/After, and the three product modes.

These interfaces keep host integration, analysis policy, rendering, persistence,
and UI independently testable.

## Verification gates

### Automated

- existing core unit and sanitizer tests;
- pairwise and consensus tests covering timing, pitch, silence, breaths,
  missing words, outliers, short clips, and incompatible inputs;
- deterministic render hashes for fixed engine/model versions;
- cancellation and stale-generation publication tests;
- real-time renderer tests proving no allocation, blocking, or file access;
- archive round-trip and incompatible-version tests;
- official ARA Test Host object lifecycle, role assignment, playback-region,
  persistence, and renderer tests;
- Apple Silicon VST3 bundle, factory, signature, and architecture validation.

### Physical Studio One acceptance

On the supported Studio One version and Apple Silicon macOS:

1. select events across multiple tracks and apply DUBL as ARA/Event FX;
2. confirm automatic analysis without capture or an extra Align action;
3. continue transport playback safely during analysis;
4. confirm atomic switch to processed playback;
5. verify the DUBL editor redraws all processed waveforms in place;
6. audition the full group with Before and After;
7. edit, trim, move, add, and remove source events and verify invalidation;
8. Undo and Redo the audible state;
9. save, close, and reopen the project and verify restoration;
10. run repeated apply/remove/reopen cycles without crashes or stale audio.

### Listening benchmark

Use licensed lead/double sessions, with Russian vocals prioritized and English
included as a portability check. Compare identical excerpts against VocAlign
and Revoice using blinded listening plus measurable timing/pitch deviation.
Release requires no regression in consonant clarity, breath naturalness,
formants, vibrato, stereo/phase behavior, and artifact rate. Speed is measured
from ARA assignment to a fully published group, not merely analysis start.

## v1 scope and deferred work

v1 supports Apple Silicon macOS, Studio One, ARA/VST3, offline local analysis,
two or more vocal events, unified in-editor waveforms, group Before/After, the
three modes, persistence, and host Undo.

Deferred until those gates pass: Windows, other DAWs, cloud processing, live
recording correction, non-vocal material, destructive host-event replacement,
manual note editing, harmony remapping, and a full custom DAW.
