#include <dubl/time_renderer.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace dubl {

RenderResult renderTimeWarp(const AudioBuffer& input, const WarpPlan& plan) {
  if (!isSupportedSampleRate(input.sample_rate) || input.samples.empty() ||
      std::ranges::any_of(input.samples, [](float sample) { return !std::isfinite(sample); })) {
    return {.error = RenderError::invalid_audio};
  }
  if (plan.points.empty() ||
      std::ranges::any_of(plan.points, [](const WarpPoint& point) {
        return !std::isfinite(point.source_seconds) ||
               !std::isfinite(point.target_seconds) || point.source_seconds < 0.0 ||
               point.target_seconds < 0.0;
      }) ||
      !std::ranges::is_sorted(plan.points, {}, &WarpPoint::source_seconds) ||
      !std::ranges::is_sorted(plan.points, {}, &WarpPoint::target_seconds)) {
    return {.error = RenderError::invalid_plan};
  }

  if (std::ranges::all_of(plan.points, [](const WarpPoint& point) { return point.identity; })) {
    return {.audio = input, .error = RenderError::none};
  }

  AudioBuffer output{.sample_rate = input.sample_rate};
  output.samples.resize(input.samples.size());
  for (std::size_t index = 0; index < output.samples.size(); ++index) {
    const double target_time = static_cast<double>(index) / input.sample_rate;
    if (target_time < plan.points.front().target_seconds ||
        target_time > plan.points.back().target_seconds) {
      output.samples[index] = input.samples[index];
      continue;
    }

    const auto upper = std::ranges::upper_bound(
        plan.points, target_time, {}, &WarpPoint::target_seconds);
    if (upper == plan.points.begin() || upper == plan.points.end()) {
      output.samples[index] = input.samples[index];
      continue;
    }
    const auto& right = *upper;
    const auto& left = *(upper - 1);
    double source_time = target_time;
    const double target_span = right.target_seconds - left.target_seconds;
    if (!(left.identity && right.identity) && target_span > 1.0e-12) {
      const double ratio = (target_time - left.target_seconds) / target_span;
      source_time = left.source_seconds + ratio *
                    (right.source_seconds - left.source_seconds);
    }
    const double source_position = std::clamp(
        source_time * input.sample_rate, 0.0,
        static_cast<double>(input.samples.size() - 1));
    const auto first = static_cast<std::size_t>(std::floor(source_position));
    const auto second = std::min(first + 1, input.samples.size() - 1);
    const float fraction = static_cast<float>(source_position - first);
    output.samples[index] = input.samples[first] * (1.0F - fraction) +
                            input.samples[second] * fraction;
  }
  return {.audio = std::move(output), .error = RenderError::none};
}

}  // namespace dubl
