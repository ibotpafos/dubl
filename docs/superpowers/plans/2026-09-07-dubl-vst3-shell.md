# DUBL macOS VST3 shell plan

Issue: #7

1. Add an opt-in, pinned JUCE 8.0.14 build so the existing core build remains
   small and reliable.
2. Build an Apple Silicon VST3 effect with safe pass-through audio, persisted
   Natural/Tight/Locked mode, and a single Align action.
3. Add a smoke test for parameter/state behavior and validate the generated
   bundle with JUCE's VST3 module-info helper and macOS bundle inspection.
4. Document local installation and clearly state that this shell does not yet
   consume Studio One event audio through ARA.

The shell is a host-integration milestone, not the final alignment workflow.
