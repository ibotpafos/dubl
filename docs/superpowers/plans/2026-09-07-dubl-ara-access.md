# DUBL ARA source-access plan

Issue: #9

1. Fetch and pin the official ARA SDK 2.3.0 including its required submodules.
2. Enable JUCE's ARA VST3 wrapper and add a minimal document-controller
   specialisation with safe empty persistence.
3. Record audio-source and playback-region creation so the UI can distinguish
   ordinary VST3 loading from real ARA event access.
4. Build and validate the ARA factory metadata, lifecycle, bundle architecture,
   and processor regression tests on macOS and CI.
5. Install the bundle and observe Studio One discovering it as an ARA-capable
   Event FX before claiming host integration.

This slice proves source/region access. Host-owned offline replacement audio is
deliberately a later slice after lifecycle behavior is observed.
