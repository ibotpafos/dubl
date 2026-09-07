#include <dubl/pitch_renderer.hpp>

#include <signalsmith-stretch/signalsmith-stretch.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

namespace dubl {

RenderResult renderPitch(const AudioBuffer& input, const float factor,
                         const float formant_base_hz) {
  if (input.sample_rate <= 0 || input.samples.empty() ||
      !std::isfinite(factor) || factor < 0.9439F || factor > 1.0595F ||
      !std::isfinite(formant_base_hz) || formant_base_hz <= 0.0F) {
    return {.error = RenderError::invalid_audio};
  }
  if (std::ranges::any_of(input.samples,
                          [](float value) { return !std::isfinite(value); })) {
    return {.error = RenderError::invalid_audio};
  }
  if (std::abs(factor - 1.0F) < 0.0005F) {
    return {.audio = input};
  }
  if (input.samples.size() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
    return {.error = RenderError::invalid_audio};
  }

  signalsmith::stretch::SignalsmithStretch<float> stretch;
  stretch.presetDefault(1, input.sample_rate);
  stretch.setTransposeFactor(factor);
  stretch.setFormantBase(formant_base_hz / static_cast<float>(input.sample_rate));
  stretch.setFormantFactor(1.0F, true);

  std::vector<std::vector<float>> source{input.samples};
  std::vector<std::vector<float>> destination(
      1, std::vector<float>(input.samples.size(), 0.0F));
  const int sample_count = static_cast<int>(input.samples.size());
  if (!stretch.exact(source, sample_count, destination, sample_count)) {
    return {.error = RenderError::invalid_audio};
  }
  for (auto& sample : destination.front()) sample = std::clamp(sample, -1.0F, 1.0F);
  return {.audio = AudioBuffer{.sample_rate = input.sample_rate,
                              .samples = std::move(destination.front())}};
}

}  // namespace dubl
