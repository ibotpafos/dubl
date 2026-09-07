Adds a native macOS application for one lead and multiple vocal doubles, with
WAV drag-and-drop, waveform lanes, batch rendering, individual/group audition,
and export to a new folder. The offline engine is bundled in the app.

Verification: native app builds and opens; executable integration test processes
four synthetic doubles through the same batch runner used by the app, verifies
output lengths and unchanged source bytes, and verifies rollback after a missing
input. CI now builds and tests the standalone app.

Remaining: physical drag-and-drop/export acceptance and vocal listening tests.
DSP is the existing experimental timing/global-pitch baseline. ARA is separate.

Refs #11; keep the issue open until UI acceptance is complete.
