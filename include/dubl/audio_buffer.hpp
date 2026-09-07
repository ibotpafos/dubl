#pragma once

#include <vector>

namespace dubl {

struct AudioBuffer {
  int sample_rate{};
  std::vector<float> samples;
};

inline bool isSupportedSampleRate(const int sample_rate) noexcept {
  return sample_rate == 44100 || sample_rate == 48000;
}

}  // namespace dubl
