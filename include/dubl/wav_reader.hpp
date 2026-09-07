#pragma once

#include <dubl/audio_buffer.hpp>

#include <filesystem>
#include <optional>

namespace dubl {

enum class WavError {
  none,
  file_open,
  malformed_riff,
  unsupported_format,
  unsupported_channels,
  unsupported_sample_rate,
  truncated,
};

struct WavLoadResult {
  std::optional<AudioBuffer> audio;
  WavError error{WavError::none};
};

WavLoadResult loadMonoWav(const std::filesystem::path& path);

}  // namespace dubl
