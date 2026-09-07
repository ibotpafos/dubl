# DUBL standalone design QA

- Source visual truth: conversation attachment `Studio Pro Appshot 2026-09-07T03-17-00.176Z.png` (Studio Pro arrangement view), plus the earlier VocAlign Pro panel attachment.
- Implementation evidence: native DUBL window captured through macOS UI automation on 2026-09-07.
- Viewport: DUBL content approximately 980 x 654 px; source capture 1335 x 767 px. The comparison used the full desktop layouts at their native densities; DUBL is intentionally narrower because it is a standalone tool rather than a full DAW.
- State: five selected synthetic WAV tracks, before and processed states; Natural mode.

## Full-view comparison

The implementation now follows the selected DAW reference rather than the old Guide/Dub/Output stack: one arrangement grid, a shared ruler, track headers on the left, colored audio clips with real waveform data, selection controls, and a persistent processing panel on the right. The standalone keeps the dense dark-purple visual language of the earlier plug-in reference while removing its separate output lane.

## Focused comparison

- Track field: source has compact headers, vertical time grid, colored clips, and waveforms. DUBL has the same hierarchy and keeps every loaded item in a single timeline.
- Processing state: after Align All, the same clip rows redraw from the rendered WAVs and show `ALIGNED`; no duplicate output tracks are introduced.
- Controls: Before/After, per-track playback, selection, removal, mode selection, formant status, and Align All are visible and functional.
- Right panel: proportions and density match the reference closely, but unsupported detailed VocAlign controls were intentionally omitted instead of shown as fake controls.

## Required fidelity surfaces

- Fonts and typography: native SF typography, compact uppercase labels, monospaced ruler; hierarchy is readable at the target window size.
- Spacing and layout rhythm: fixed track header, aligned timeline rows, 72 px lane rhythm, persistent 270 px control panel, no clipped primary controls.
- Colors and visual tokens: near-black purple surfaces, subdued borders, distinct per-track accents, yellow timing, orange pitch, and violet primary action.
- Image and asset quality: no decorative raster assets are required. SF Symbols provide standard controls; waveform geometry is derived from the actual WAV samples.
- Copy and content: equal `ТРЕК` naming replaces Lead/Dub/Output terminology. The internal first-selected reference is disclosed in the settings panel.

## Comparison history

1. Initial build used separate Lead, Doubles, and Output lanes. This was a P1 mismatch with the clarified one-field workflow.
2. The layout was replaced by a DAW grid, but output rows initially disappeared because input and output `ForEach` collections reused identifiers. This was fixed and physically rechecked with four rendered results.
3. The clarified design removed output rows entirely. Track selection and an in-place Before/After redraw were added and physically verified: `ALIGNED` appeared on four processed tracks, disappeared on Before, and returned on After.
4. Track headers were widened and labels shortened to prevent the visible truncation found in the first loaded-state capture.

## Findings

No actionable P0, P1, or P2 visual mismatches remain for the clarified product direction.

## Follow-up polish

- P3: derive the ruler scale and clip width from the longest loaded file rather than the current fixed overview scale.
- P3: add a moving playhead during synchronized audition.

final result: passed
