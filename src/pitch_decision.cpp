#include <dubl/pitch_decision.hpp>

#include <algorithm>
#include <cmath>
#include <vector>

namespace dubl {

PitchCorrection choosePitchCorrection(const WarpPlan& plan, const PitchMode mode,
                                      const float minimum_confidence) {
  PitchCorrection result;
  if (!std::isfinite(plan.confidence) || plan.confidence < minimum_confidence) {
    return result;
  }

  std::vector<float> evidence;
  evidence.reserve(plan.points.size());
  for (const auto& point : plan.points) {
    if (!point.identity && std::isfinite(point.pitch_ratio) &&
        point.pitch_ratio > 0.0F) {
      evidence.push_back(std::clamp(point.pitch_ratio, 0.9439F, 1.0595F));
    }
  }
  result.evidence_points = evidence.size();
  if (evidence.size() < 3) return result;

  const auto middle = evidence.begin() + static_cast<std::ptrdiff_t>(evidence.size() / 2);
  std::nth_element(evidence.begin(), middle, evidence.end());
  const float median = *middle;
  const float strength = mode == PitchMode::natural ? 0.35F
                         : mode == PitchMode::tight ? 0.70F
                                                   : 1.0F;
  result.factor = std::clamp(std::pow(median, strength), 0.9439F, 1.0595F);
  result.applied = std::abs(result.factor - 1.0F) >= 0.0005F;
  if (!result.applied) result.factor = 1.0F;
  return result;
}

}  // namespace dubl
