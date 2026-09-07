#pragma once

#include <dubl/warp_plan.hpp>

#include <cstddef>

namespace dubl {

enum class PitchMode { natural, tight, locked };

struct PitchCorrection {
  float factor{1.0F};
  std::size_t evidence_points{};
  bool applied{};
};

PitchCorrection choosePitchCorrection(const WarpPlan& plan, PitchMode mode,
                                      float minimum_confidence = 0.70F);

}  // namespace dubl
