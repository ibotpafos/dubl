#pragma once

#include <dubl/audio_buffer.hpp>
#include <dubl/warp_plan.hpp>

#include <optional>

namespace dubl {

enum class RenderError { none, invalid_audio, invalid_plan };

struct RenderResult {
  std::optional<AudioBuffer> audio;
  RenderError error{RenderError::none};
};

RenderResult renderTimeWarp(const AudioBuffer& input, const WarpPlan& plan);

}  // namespace dubl
