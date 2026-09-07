#pragma once

#include <dubl/audio_buffer.hpp>
#include <dubl/time_renderer.hpp>

namespace dubl {

RenderResult renderPitch(const AudioBuffer& input, float factor,
                         float formant_base_hz);

}  // namespace dubl
