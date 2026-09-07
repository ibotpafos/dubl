Adds a native macOS application with a unified DAW-style track area, multi-WAV
drag-and-drop, selectable tracks, in-place processed waveforms, batch rendering,
instant Before/After group audition, and export to a new folder. The first
selected track is the internal reference, without a separate lead/output lane.
The offline engine is bundled in the app.

Verification: native app builds and opens; executable integration test processes
four synthetic doubles through the same batch runner used by the app, verifies
output lengths and unchanged source bytes, and verifies rollback after a missing
input. CI now builds and tests the standalone app.

Remaining: physical export acceptance and real-vocal listening tests.
DSP is the existing experimental timing/global-pitch baseline. ARA is separate.

Refs #11; keep the issue open until UI acceptance is complete.
