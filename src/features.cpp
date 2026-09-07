#include <dubl/features.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <numbers>
#include <vector>

namespace dubl {

std::vector<FrameFeature> extractFeatures(const AudioBuffer& audio,
                                          const int hop_samples) {
  if (!isSupportedSampleRate(audio.sample_rate) || audio.samples.empty() ||
      hop_samples <= 0) {
    return {};
  }

  const auto window_size = static_cast<std::size_t>(
      std::max(32, static_cast<int>(std::lround(audio.sample_rate * 0.040))));
  const auto hop = static_cast<std::size_t>(std::max(
      1, static_cast<int>(std::lround(hop_samples * audio.sample_rate / 48000.0))));
  const auto min_lag = std::max(1, audio.sample_rate / 500);
  const auto max_lag = std::min(static_cast<int>(window_size) - 2,
                                audio.sample_rate / 80);

  std::vector<FrameFeature> frames;
  if (audio.samples.size() < window_size) return frames;
  frames.reserve(1 + (audio.samples.size() - window_size) / hop);
  float previous_rms = 0.0F;
  std::vector<double> windowed(window_size);

  for (std::size_t start = 0; start + window_size <= audio.samples.size();
       start += hop) {
    double energy = 0.0;
    double mean = 0.0;
    for (std::size_t i = 0; i < window_size; ++i) {
      mean += audio.samples[start + i];
      energy += static_cast<double>(audio.samples[start + i]) *
                audio.samples[start + i];
    }
    mean /= static_cast<double>(window_size);
    const auto rms = static_cast<float>(std::sqrt(energy / window_size));

    for (std::size_t i = 0; i < window_size; ++i) {
      const double hann = 0.5 - 0.5 * std::cos(
          2.0 * std::numbers::pi * static_cast<double>(i) /
          static_cast<double>(window_size - 1));
      windowed[i] = (audio.samples[start + i] - mean) * hann;
    }

    float best_correlation = 0.0F;
    int best_lag = 0;
    if (rms >= 0.01F) {
      for (int lag = min_lag; lag <= max_lag; ++lag) {
        double numerator = 0.0;
        double left_energy = 0.0;
        double right_energy = 0.0;
        const auto available = window_size - static_cast<std::size_t>(lag);
        for (std::size_t i = 0; i < available; ++i) {
          const auto left = windowed[i];
          const auto right = windowed[i + static_cast<std::size_t>(lag)];
          numerator += left * right;
          left_energy += left * left;
          right_energy += right * right;
        }
        const auto denominator = std::sqrt(left_energy * right_energy);
        const auto correlation = denominator > 1.0e-12
                                     ? static_cast<float>(numerator / denominator)
                                     : 0.0F;
        if (correlation > best_correlation) {
          best_correlation = correlation;
          best_lag = lag;
        }
      }
    }

    const bool voiced = rms >= 0.01F && best_correlation >= 0.55F && best_lag > 0;
    frames.push_back({
        .time_seconds = static_cast<double>(start + window_size / 2) /
                        audio.sample_rate,
        .rms = rms,
        .onset = std::max(0.0F, rms - previous_rms),
        .f0_hz = voiced ? static_cast<float>(audio.sample_rate) / best_lag : 0.0F,
        .voiced = voiced,
    });
    previous_rms = rms;
  }
  return frames;
}

}  // namespace dubl
