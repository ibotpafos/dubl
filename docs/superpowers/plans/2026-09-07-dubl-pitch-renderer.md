# DUBL confidence-aware pitch renderer plan

Issue: #5

## Product slice

Turn the merged timing-only CLI into a conservative timing-and-pitch offline
product. Reuse the pitch evidence already stored in `WarpPlan`, apply no shift
when evidence is insufficient, and preserve the rendered duration.

## Implementation

1. Add a tested pitch decision layer with Natural, Tight, and Locked strengths.
   It rejects malformed plans, ignores identity points, clamps correction to the
   existing safe range, and returns an explicit applied/skipped decision.
2. Integrate the pinned MIT-licensed Signalsmith Stretch renderer with formant
   compensation. Keep this dependency outside `dubl_core` so analysis remains
   lightweight and deterministic.
3. Extend `dubl_render` with `--mode natural|tight|locked`, timing followed by
   fixed-duration pitch rendering, and a truthful capability report.
4. Verify identity behavior, pitch direction, duration, CLI safety, clean
   Release compilation, sanitizers, and macOS CI.

## Quality boundary

This milestone provides bounded global pitch correction derived from confident
aligned regions. Time-varying note-by-note correction, learned artifact
detection, and corpus listening gates remain required before a production
quality claim.
