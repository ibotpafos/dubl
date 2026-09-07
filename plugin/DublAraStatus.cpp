#include "DublAraStatus.hpp"

std::atomic<std::size_t> DublAraStatus::audio_sources{0};
std::atomic<std::size_t> DublAraStatus::playback_regions{0};

void DublAraStatus::observedAudioSource() noexcept { ++audio_sources; }
void DublAraStatus::observedPlaybackRegion() noexcept { ++playback_regions; }
std::size_t DublAraStatus::audioSourcesObserved() noexcept { return audio_sources.load(); }
std::size_t DublAraStatus::playbackRegionsObserved() noexcept { return playback_regions.load(); }
