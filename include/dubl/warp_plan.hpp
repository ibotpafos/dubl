#pragma once

#include <dubl/alignment.hpp>

#include <span>
#include <vector>

namespace dubl {

struct WarpPoint {
  double source_seconds{};
  double target_seconds{};
  float pitch_ratio{1.0F};
  bool identity{true};
};

struct WarpPlan {
  std::vector<WarpPoint> points;
  float confidence{};
};

WarpPlan makeSafeWarpPlan(const AlignmentResult& alignment,
                          std::span<const FrameFeature> lead,
                          std::span<const FrameFeature> dub,
                          float minimum_confidence = 0.70F);

}  // namespace dubl
