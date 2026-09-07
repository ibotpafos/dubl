#include <dubl/alignment.hpp>
#include "test_support.hpp"

#include <algorithm>
#include <vector>

namespace {

std::vector<dubl::FrameFeature> phrase(int count) {
  std::vector<dubl::FrameFeature> frames;
  for (int i = 0; i < count; ++i) {
    frames.push_back({.time_seconds = i * 0.01,
                      .rms = 0.25F,
                      .onset = i % 12 == 0 ? 0.8F : 0.02F,
                      .f0_hz = 180.0F + static_cast<float>((i / 10) % 3) * 20.0F,
                      .voiced = true});
  }
  return frames;
}

}  // namespace

int main() {
  const auto lead = phrase(80);
  auto delayed = phrase(80);
  for (auto& frame : delayed) frame.time_seconds += 0.03;
  const auto aligned = dubl::alignFeatures(lead, delayed, 30);
  REQUIRE(!aligned.points.empty());
  REQUIRE(aligned.overall_confidence > 0.90F);
  for (std::size_t i = 1; i < aligned.points.size(); ++i) {
    REQUIRE(aligned.points[i].double_frame >= aligned.points[i - 1].double_frame);
    REQUIRE(aligned.points[i].lead_frame >= aligned.points[i - 1].lead_frame);
  }

  auto unrelated = phrase(80);
  for (auto& frame : unrelated) {
    frame.onset = 1.0F - frame.onset;
    frame.f0_hz = 420.0F;
    frame.voiced = false;
  }
  REQUIRE(dubl::alignFeatures(lead, unrelated, 30).overall_confidence < 0.70F);
}
