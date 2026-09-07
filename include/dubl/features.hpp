#pragma once

#include <dubl/audio_buffer.hpp>

#include <vector>

namespace dubl {

struct FrameFeature {
  double time_seconds{};
  float rms{};
  float onset{};
  float f0_hz{};
  bool voiced{};
};

std::vector<FrameFeature> extractFeatures(const AudioBuffer& audio,
                                          int hop_samples = 480);

}  // namespace dubl
