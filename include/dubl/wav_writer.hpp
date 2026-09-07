#pragma once

#include <dubl/audio_buffer.hpp>

#include <filesystem>

namespace dubl {

enum class WavWriteError {
  none,
  destination_exists,
  invalid_audio,
  open_failed,
  write_failed,
  publish_failed,
};

struct WavWriteResult {
  WavWriteError error{WavWriteError::none};
};

WavWriteResult writeMonoPcm16Wav(const std::filesystem::path& path,
                                 const AudioBuffer& audio);

}  // namespace dubl
