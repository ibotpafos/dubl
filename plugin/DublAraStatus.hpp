#pragma once

#include <atomic>
#include <cstddef>

struct DublAraStatus {
  static void observedAudioSource() noexcept;
  static void observedPlaybackRegion() noexcept;
  static std::size_t audioSourcesObserved() noexcept;
  static std::size_t playbackRegionsObserved() noexcept;

 private:
  static std::atomic<std::size_t> audio_sources;
  static std::atomic<std::size_t> playback_regions;
};
