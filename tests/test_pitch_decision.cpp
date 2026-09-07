#include <dubl/pitch_decision.hpp>
#include "test_support.hpp"

#include <cmath>

int main() {
  dubl::WarpPlan plan{.confidence = 0.95F};
  plan.points = {
      {.pitch_ratio = 1.0F, .identity = true},
      {.pitch_ratio = 1.04F, .identity = false},
      {.pitch_ratio = 1.05F, .identity = false},
      {.pitch_ratio = 1.20F, .identity = false},
      {.pitch_ratio = 1.0F, .identity = true},
  };

  const auto natural = dubl::choosePitchCorrection(plan, dubl::PitchMode::natural);
  const auto tight = dubl::choosePitchCorrection(plan, dubl::PitchMode::tight);
  const auto locked = dubl::choosePitchCorrection(plan, dubl::PitchMode::locked);
  REQUIRE(natural.applied);
  REQUIRE(natural.evidence_points == 3);
  REQUIRE(natural.factor > 1.0F);
  REQUIRE(natural.factor < tight.factor);
  REQUIRE(tight.factor < locked.factor);
  REQUIRE(locked.factor <= 1.0595F);

  plan.confidence = 0.4F;
  const auto unsafe = dubl::choosePitchCorrection(plan, dubl::PitchMode::locked);
  REQUIRE(!unsafe.applied);
  REQUIRE(std::abs(unsafe.factor - 1.0F) < 0.0001F);
}
