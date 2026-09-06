# Contributing to ДУБЛЬ / DUBL

Thanks for helping make vocal alignment natural and dependable.

## Principles

- Preserve the singer's character before chasing a perfectly rigid alignment.
- Keep audio and model inference local. Do not submit session audio, vocals,
  credentials, or unlicensed datasets in issues, commits, or pull requests.
- Reproduce a defect with synthetic or explicitly consented material.
- Treat any audible glitch, altered lyric, clipped output, lost host state, or
  failed Undo/Restore as a release-blocking issue.

## Before opening a pull request

1. Keep changes narrow and add a regression test where an automated test is
   possible.
2. State the host, macOS version, Studio One version and plug-in format used.
3. For audible work, include a consent-safe, minimal reproduction and describe
   the expected and observed behaviour rather than uploading source sessions.
4. Do not add a model or dependency until its licence, provenance and local
   inference behaviour have been reviewed.

The repository is in design and benchmark preparation. Build instructions will
arrive with the first reproducible core prototype; no non-working command is
presented as a supported build yet.
