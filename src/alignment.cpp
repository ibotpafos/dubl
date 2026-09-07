#include <dubl/alignment.hpp>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <vector>

namespace dubl {
namespace {

float frameCost(const FrameFeature& lead, const FrameFeature& dub) {
  const float onset = std::min(1.0F, std::abs(lead.onset - dub.onset));
  float pitch = 0.0F;
  if (lead.voiced && dub.voiced && lead.f0_hz > 0.0F && dub.f0_hz > 0.0F) {
    pitch = std::min(1.0F, std::abs(std::log2(lead.f0_hz / dub.f0_hz)));
  }
  const float voicing = lead.voiced == dub.voiced ? 0.0F : 1.0F;
  return 0.55F * onset + 0.30F * pitch + 0.15F * voicing;
}

float evidenceRatio(const std::span<const FrameFeature> frames) {
  const auto informative = std::ranges::count_if(frames, [](const FrameFeature& frame) {
    return frame.voiced || frame.rms >= 0.02F || frame.onset >= 0.05F;
  });
  return static_cast<float>(informative) / static_cast<float>(frames.size());
}

}  // namespace

AlignmentResult alignFeatures(const std::span<const FrameFeature> lead,
                              const std::span<const FrameFeature> dub,
                              const int max_frame_offset) {
  if (lead.empty() || dub.empty() || max_frame_offset < 0) return {};
  const std::size_t rows = dub.size();
  const std::size_t columns = lead.size();
  const float infinity = std::numeric_limits<float>::infinity();
  struct BandRow {
    std::size_t low{};
    std::vector<float> costs;
    std::vector<unsigned char> parents;
  };
  std::vector<BandRow> band;
  band.reserve(rows);
  for (std::size_t row = 0; row < rows; ++row) {
    const auto low = row > static_cast<std::size_t>(max_frame_offset)
                         ? row - static_cast<std::size_t>(max_frame_offset)
                         : 0;
    const auto high = std::min(columns - 1,
                               row + static_cast<std::size_t>(max_frame_offset));
    if (low > high) return {};
    const auto width = high - low + 1;
    band.push_back({.low = low,
                    .costs = std::vector<float>(width, infinity),
                    .parents = std::vector<unsigned char>(width, 0)});
  }
  const auto getCost = [&](std::size_t row, std::size_t column) {
    if (row >= band.size() || column < band[row].low ||
        column >= band[row].low + band[row].costs.size()) {
      return infinity;
    }
    return band[row].costs[column - band[row].low];
  };

  for (std::size_t row = 0; row < rows; ++row) {
    const auto low = band[row].low;
    const auto high = low + band[row].costs.size() - 1;
    for (std::size_t column = low; column <= high; ++column) {
      const float local = frameCost(lead[column], dub[row]);
      if (row == 0 && column == 0) {
        band[row].costs[column - low] = local;
        continue;
      }
      float best = infinity;
      unsigned char direction = 0;
      if (row > 0 && column > 0 && getCost(row - 1, column - 1) < best) {
        best = getCost(row - 1, column - 1);
        direction = 1;
      }
      if (row > 0 && getCost(row - 1, column) + 0.08F < best) {
        best = getCost(row - 1, column) + 0.08F;
        direction = 2;
      }
      if (column > 0 && getCost(row, column - 1) + 0.08F < best) {
        best = getCost(row, column - 1) + 0.08F;
        direction = 3;
      }
      if (std::isfinite(best)) {
        band[row].costs[column - low] = best + local;
        band[row].parents[column - low] = direction;
      }
    }
  }

  const float final_cost = getCost(rows - 1, columns - 1);
  if (!std::isfinite(final_cost)) return {};
  AlignmentResult result;
  std::size_t row = rows - 1;
  std::size_t column = columns - 1;
  for (;;) {
    const float local = frameCost(lead[column], dub[row]);
    result.points.push_back({.double_frame = static_cast<int>(row),
                             .lead_frame = static_cast<int>(column),
                             .confidence = 1.0F - std::min(1.0F, local)});
    if (row == 0 && column == 0) break;
    switch (band[row].parents[column - band[row].low]) {
      case 1: --row; --column; break;
      case 2: --row; break;
      case 3: --column; break;
      default: return {};
    }
  }
  std::ranges::reverse(result.points);
  const float mean_cost = final_cost / static_cast<float>(result.points.size());
  const float evidence = std::min(evidenceRatio(lead), evidenceRatio(dub));
  result.overall_confidence = (1.0F - std::min(1.0F, mean_cost)) * evidence;
  return result;
}

}  // namespace dubl
