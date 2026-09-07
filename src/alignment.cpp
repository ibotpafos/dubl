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

}  // namespace

AlignmentResult alignFeatures(const std::span<const FrameFeature> lead,
                              const std::span<const FrameFeature> dub,
                              const int max_frame_offset) {
  if (lead.empty() || dub.empty() || max_frame_offset < 0) return {};
  const std::size_t rows = dub.size();
  const std::size_t columns = lead.size();
  const float infinity = std::numeric_limits<float>::infinity();
  std::vector<float> cost(rows * columns, infinity);
  std::vector<unsigned char> parent(rows * columns, 0);
  const auto index = [columns](std::size_t row, std::size_t column) {
    return row * columns + column;
  };

  for (std::size_t row = 0; row < rows; ++row) {
    const auto low = row > static_cast<std::size_t>(max_frame_offset)
                         ? row - static_cast<std::size_t>(max_frame_offset)
                         : 0;
    const auto high = std::min(columns - 1,
                               row + static_cast<std::size_t>(max_frame_offset));
    for (std::size_t column = low; column <= high; ++column) {
      const float local = frameCost(lead[column], dub[row]);
      if (row == 0 && column == 0) {
        cost[index(row, column)] = local;
        continue;
      }
      float best = infinity;
      unsigned char direction = 0;
      if (row > 0 && column > 0 && cost[index(row - 1, column - 1)] < best) {
        best = cost[index(row - 1, column - 1)];
        direction = 1;
      }
      if (row > 0 && cost[index(row - 1, column)] + 0.08F < best) {
        best = cost[index(row - 1, column)] + 0.08F;
        direction = 2;
      }
      if (column > 0 && cost[index(row, column - 1)] + 0.08F < best) {
        best = cost[index(row, column - 1)] + 0.08F;
        direction = 3;
      }
      if (std::isfinite(best)) {
        cost[index(row, column)] = best + local;
        parent[index(row, column)] = direction;
      }
    }
  }

  if (!std::isfinite(cost[index(rows - 1, columns - 1)])) return {};
  AlignmentResult result;
  std::size_t row = rows - 1;
  std::size_t column = columns - 1;
  for (;;) {
    const float local = frameCost(lead[column], dub[row]);
    result.points.push_back({.double_frame = static_cast<int>(row),
                             .lead_frame = static_cast<int>(column),
                             .confidence = 1.0F - std::min(1.0F, local)});
    if (row == 0 && column == 0) break;
    switch (parent[index(row, column)]) {
      case 1: --row; --column; break;
      case 2: --row; break;
      case 3: --column; break;
      default: return {};
    }
  }
  std::ranges::reverse(result.points);
  const float mean_cost = cost[index(rows - 1, columns - 1)] /
                          static_cast<float>(result.points.size());
  result.overall_confidence = 1.0F - std::min(1.0F, mean_cost);
  return result;
}

}  // namespace dubl
