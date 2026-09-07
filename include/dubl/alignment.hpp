#pragma once

#include <dubl/features.hpp>

#include <span>
#include <vector>

namespace dubl {

struct AlignmentPoint {
  int double_frame{};
  int lead_frame{};
  float confidence{};
};

struct AlignmentResult {
  std::vector<AlignmentPoint> points;
  float overall_confidence{};
};

AlignmentResult alignFeatures(std::span<const FrameFeature> lead,
                              std::span<const FrameFeature> dub,
                              int max_frame_offset = 200);

}  // namespace dubl
