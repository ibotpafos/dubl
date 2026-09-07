#include <dubl/time_renderer.hpp>
#include "test_support.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

int main() {
  const dubl::AudioBuffer audio{
      .sample_rate = 48000,
      .samples = {0.0F, 0.25F, 0.5F, 0.75F, 1.0F},
  };
  const double end = static_cast<double>(audio.samples.size() - 1) / audio.sample_rate;
  const dubl::WarpPlan identity{
      .points = {{0.0, 0.0, 1.0F, true}, {end, end, 1.0F, true}},
      .confidence = 1.0F,
  };
  const auto identity_result = dubl::renderTimeWarp(audio, identity);
  REQUIRE(identity_result.audio.has_value());
  REQUIRE(identity_result.audio->samples == audio.samples);

  const dubl::WarpPlan warped{
      .points = {{0.0, 0.0, 1.0F, true},
                 {end * 0.4, end * 0.6, 1.0F, false},
                 {end, end, 1.0F, true}},
      .confidence = 0.9F,
  };
  const auto warped_result = dubl::renderTimeWarp(audio, warped);
  REQUIRE(warped_result.audio.has_value());
  REQUIRE(warped_result.audio->samples.size() == audio.samples.size());
  REQUIRE(std::ranges::all_of(warped_result.audio->samples,
                              [](float sample) { return std::isfinite(sample); }));

  auto invalid = warped;
  invalid.points[1].target_seconds = -0.1;
  REQUIRE(dubl::renderTimeWarp(audio, invalid).error == dubl::RenderError::invalid_plan);
}
