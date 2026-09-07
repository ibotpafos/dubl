#include <dubl/warp_plan.hpp>
#include "test_support.hpp"

#include <algorithm>
#include <vector>

namespace {

std::vector<dubl::FrameFeature> features(float pitch, double offset = 0.0) {
  std::vector<dubl::FrameFeature> result;
  for (int i = 0; i < 20; ++i) {
    result.push_back({.time_seconds = offset + i * 0.01,
                      .rms = 0.3F,
                      .onset = i == 0 ? 0.7F : 0.0F,
                      .f0_hz = pitch,
                      .voiced = true});
  }
  return result;
}

dubl::AlignmentResult alignment(float confidence) {
  dubl::AlignmentResult result{.overall_confidence = confidence};
  for (int i = 0; i < 20; ++i) {
    result.points.push_back({.double_frame = i, .lead_frame = i,
                             .confidence = confidence});
  }
  return result;
}

}  // namespace

int main() {
  const auto lead = features(220.0F);
  const auto dub = features(200.0F, 0.02);

  const auto unsafe = dubl::makeSafeWarpPlan(alignment(0.50F), lead, dub);
  REQUIRE(!unsafe.points.empty());
  REQUIRE(std::ranges::all_of(unsafe.points,
                             [](const auto& point) { return point.identity; }));

  const auto safe = dubl::makeSafeWarpPlan(alignment(0.95F), lead, dub);
  REQUIRE(std::ranges::is_sorted(safe.points, {}, &dubl::WarpPoint::source_seconds));
  REQUIRE(std::ranges::is_sorted(safe.points, {}, &dubl::WarpPoint::target_seconds));
  REQUIRE(std::ranges::all_of(safe.points, [](const auto& point) {
    return point.pitch_ratio >= 0.9439F && point.pitch_ratio <= 1.0595F;
  }));
  REQUIRE(safe.points.front().source_seconds == safe.points.front().target_seconds);
  REQUIRE(safe.points.back().source_seconds == safe.points.back().target_seconds);
}
