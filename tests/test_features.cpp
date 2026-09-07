#include <dubl/features.hpp>
#include "test_support.hpp"

#include <algorithm>
#include <cmath>
#include <numbers>
#include <vector>

namespace {

dubl::AudioBuffer sineBuffer(int sample_rate, float frequency, float seconds) {
  dubl::AudioBuffer audio{.sample_rate = sample_rate};
  const auto count = static_cast<std::size_t>(sample_rate * seconds);
  audio.samples.resize(count);
  for (std::size_t i = 0; i < count; ++i) {
    audio.samples[i] = 0.5F * std::sin(2.0F * std::numbers::pi_v<float> * frequency *
                                     static_cast<float>(i) / sample_rate);
  }
  return audio;
}

float medianVoicedF0(std::vector<dubl::FrameFeature> frames) {
  std::vector<float> values;
  for (const auto& frame : frames) if (frame.voiced) values.push_back(frame.f0_hz);
  std::ranges::sort(values);
  return values.at(values.size() / 2);
}

}  // namespace

int main() {
  const auto frames = dubl::extractFeatures(sineBuffer(44100, 220.0F, 1.0F));
  REQUIRE(std::ranges::count_if(frames, [](const auto& frame) { return frame.voiced; }) > 50);
  REQUIRE(std::abs(medianVoicedF0(frames) - 220.0F) < 5.0F);

  dubl::AudioBuffer silence{.sample_rate = 44100, .samples = std::vector<float>(44100, 0.0F)};
  const auto silent_frames = dubl::extractFeatures(silence);
  REQUIRE(!silent_frames.empty());
  REQUIRE(std::ranges::none_of(silent_frames, [](const auto& frame) { return frame.voiced; }));
}
