#include <dubl/pitch_renderer.hpp>
#include "test_support.hpp"

#include <cmath>
#include <numbers>

namespace {

dubl::AudioBuffer tone(float frequency) {
  dubl::AudioBuffer audio{.sample_rate = 48000};
  audio.samples.resize(48000);
  for (std::size_t i = 0; i < audio.samples.size(); ++i) {
    audio.samples[i] = 0.25F * std::sin(
        2.0 * std::numbers::pi * frequency * static_cast<double>(i) / 48000.0);
  }
  return audio;
}

float zeroCrossingFrequency(const dubl::AudioBuffer& audio) {
  std::size_t crossings = 0;
  const std::size_t begin = 12000;
  const std::size_t end = 36000;
  for (std::size_t i = begin + 1; i < end; ++i) {
    if (audio.samples[i - 1] <= 0.0F && audio.samples[i] > 0.0F) ++crossings;
  }
  return static_cast<float>(crossings) * audio.sample_rate /
         static_cast<float>(end - begin);
}

}  // namespace

int main() {
  const auto input = tone(200.0F);
  const auto identity = dubl::renderPitch(input, 1.0F, 200.0F);
  REQUIRE(identity.audio.has_value());
  REQUIRE(identity.audio->samples == input.samples);

  const auto shifted = dubl::renderPitch(input, 1.05F, 200.0F);
  REQUIRE(shifted.audio.has_value());
  REQUIRE(shifted.audio->samples.size() == input.samples.size());
  REQUIRE(std::abs(zeroCrossingFrequency(*shifted.audio) - 210.0F) < 4.0F);

  const auto invalid = dubl::renderPitch(input, 2.0F, 200.0F);
  REQUIRE(!invalid.audio.has_value());
}
