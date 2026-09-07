#include <dubl/warp_plan.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace dubl {
namespace {

WarpPlan identityPlan(const std::span<const FrameFeature> dub, float confidence) {
  WarpPlan plan{.confidence = confidence};
  plan.points.reserve(dub.size());
  for (const auto& frame : dub) {
    plan.points.push_back({.source_seconds = frame.time_seconds,
                           .target_seconds = frame.time_seconds,
                           .pitch_ratio = 1.0F,
                           .identity = true});
  }
  return plan;
}

}  // namespace

WarpPlan makeSafeWarpPlan(const AlignmentResult& alignment,
                          const std::span<const FrameFeature> lead,
                          const std::span<const FrameFeature> dub,
                          const float minimum_confidence) {
  if (dub.empty()) return {.confidence = alignment.overall_confidence};
  if (lead.empty() || alignment.points.empty() ||
      alignment.overall_confidence < minimum_confidence) {
    return identityPlan(dub, alignment.overall_confidence);
  }

  WarpPlan plan{.confidence = alignment.overall_confidence};
  plan.points.reserve(alignment.points.size());
  const double source_end = dub.back().time_seconds;
  for (std::size_t match_index = 0; match_index < alignment.points.size(); ++match_index) {
    const auto& match = alignment.points[match_index];
    if (match.double_frame < 0 || match.lead_frame < 0 ||
        static_cast<std::size_t>(match.double_frame) >= dub.size() ||
        static_cast<std::size_t>(match.lead_frame) >= lead.size()) {
      return identityPlan(dub, 0.0F);
    }
    const auto& source = dub[static_cast<std::size_t>(match.double_frame)];
    const auto& reference = lead[static_cast<std::size_t>(match.lead_frame)];
    bool identity = match.confidence < minimum_confidence;
    float pitch_ratio = 1.0F;
    if (!identity && source.voiced && reference.voiced && source.f0_hz > 0.0F) {
      pitch_ratio = std::clamp(reference.f0_hz / source.f0_hz, 0.9439F, 1.0595F);
    }
    double target = identity ? source.time_seconds : reference.time_seconds;
    if (match_index == 0 || match_index + 1 == alignment.points.size()) {
      target = source.time_seconds;
      pitch_ratio = 1.0F;
      identity = true;
    } else if (!plan.points.empty()) {
      const auto& previous = plan.points.back();
      const double source_delta = source.time_seconds - previous.source_seconds;
      if (source_delta < 0.0) return identityPlan(dub, 0.0F);
      const double remaining = source_end - source.time_seconds;
      const double minimum_target = std::max(
          previous.target_seconds + source_delta * 0.85,
          source_end - remaining * 1.18);
      const double maximum_target = std::min(
          previous.target_seconds + source_delta * 1.18,
          source_end - remaining * 0.85);
      if (minimum_target > maximum_target) return identityPlan(dub, 0.0F);
      target = std::clamp(target, minimum_target, maximum_target);
    }
    plan.points.push_back({.source_seconds = source.time_seconds,
                           .target_seconds = target,
                           .pitch_ratio = pitch_ratio,
                           .identity = identity});
  }

  return plan;
}

}  // namespace dubl
