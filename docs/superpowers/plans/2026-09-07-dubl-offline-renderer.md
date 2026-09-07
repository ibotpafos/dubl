# DUBL Offline Renderer Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Render a lead-guided, duration-preserving vocal double to a new mono WAV from the command line.

**Architecture:** Add a deterministic inverse time-map renderer and an atomic PCM16 WAV writer to `dubl_core`. A thin `dubl_render` CLI reuses the existing analysis pipeline and refuses source overwrite or incompatible sample rates. Pitch/formant processing remains explicitly disabled until its own perceptual benchmark exists.

**Tech Stack:** C++20, CMake 3.25+, CTest, AppleClang.

**Spec:** `docs/superpowers/specs/2026-09-07-vocal-align-studio-one-design.md`

## Global Constraints

- Local and offline only; exact output frame count equals the double input.
- Reject NaN/Inf, mismatched sample rates, source overwrite and existing output.
- Low-confidence analysis produces an identity time map.
- Write to a same-directory temporary file and rename only after a complete flush.
- No pitch correction claim in this milestone.

---

### Task 1: Atomic PCM16 WAV writer

**Files:**
- Create: `include/dubl/wav_writer.hpp`
- Create: `src/wav_writer.cpp`
- Create: `tests/test_wav_writer.cpp`
- Modify: `CMakeLists.txt`

**Produces:** `WavWriteResult writeMonoPcm16Wav(const path&, const AudioBuffer&)`.

- [ ] Write tests proving exact RIFF metadata, normalized sample round-trip, refusal to overwrite, and no final file after invalid samples.
- [ ] Run `ctest -R wav_writer` and observe failure because the API is absent.
- [ ] Implement clipping, little-endian PCM16 encoding, same-directory `.dubl-tmp` publication, flush validation and non-replacing rename.
- [ ] Run the focused and full suite; expect all tests to pass.
- [ ] Commit as `feat: write aligned WAV atomically`.

### Task 2: Duration-preserving time renderer

**Files:**
- Create: `include/dubl/time_renderer.hpp`
- Create: `src/time_renderer.cpp`
- Create: `tests/test_time_renderer.cpp`
- Modify: `CMakeLists.txt`

**Produces:** `RenderResult renderTimeWarp(const AudioBuffer&, const WarpPlan&)`.

- [ ] Write tests proving identity sample equality, exact frame count, finite linear interpolation and rejection of non-monotonic plans.
- [ ] Run `ctest -R time_renderer` and observe failure because the API is absent.
- [ ] Implement piecewise-linear inversion from target time to source time and bounded sample interpolation; identity segments copy source samples exactly.
- [ ] Run focused and full tests; expect all tests to pass.
- [ ] Commit as `feat: render bounded vocal timing maps`.

### Task 3: End-to-end render CLI

**Files:**
- Create: `apps/dubl_render.cpp`
- Create: `tests/test_render_cli.cpp`
- Modify: `CMakeLists.txt`
- Modify: `README.md`

**Produces:** `dubl_render --lead lead.wav --double double.wav --output aligned.wav --report report.json`.

- [ ] Write an end-to-end test generating synthetic WAVs and asserting output existence, exact frame count, JSON schema, and refusal to overwrite either source.
- [ ] Run `ctest -R render_cli` and observe failure because the executable is absent.
- [ ] Compose load → features → alignment → plan → time render → atomic WAV/report publication with documented exit codes.
- [ ] Document build/run commands and state plainly that P1 renders timing only.
- [ ] Run clean Release `-Werror` plus ASan/UBSan and the complete CTest suite.
- [ ] Commit as `feat: add offline DUBL render product`.

## Completion evidence

- A generated WAV is readable by the existing strict reader and has the double's exact sample count.
- Every new error path has an automated test.
- CI, sanitizer suite and `git diff --check` pass before PR creation.
