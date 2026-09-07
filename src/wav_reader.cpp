#include <dubl/wav_reader.hpp>

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <limits>
#include <string_view>
#include <vector>

namespace dubl {
namespace {

std::uint16_t readU16(const std::byte* data) {
  return std::to_integer<std::uint16_t>(data[0]) |
         (std::to_integer<std::uint16_t>(data[1]) << 8U);
}

std::uint32_t readU32(const std::byte* data) {
  return std::to_integer<std::uint32_t>(data[0]) |
         (std::to_integer<std::uint32_t>(data[1]) << 8U) |
         (std::to_integer<std::uint32_t>(data[2]) << 16U) |
         (std::to_integer<std::uint32_t>(data[3]) << 24U);
}

bool tagEquals(const std::byte* data, std::string_view tag) {
  return tag.size() == 4 &&
         std::memcmp(data, tag.data(), tag.size()) == 0;
}

WavLoadResult fail(WavError error) { return {.audio = std::nullopt, .error = error}; }

}  // namespace

WavLoadResult loadMonoWav(const std::filesystem::path& path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) return fail(WavError::file_open);

  stream.seekg(0, std::ios::end);
  const auto length = stream.tellg();
  if (length < 12) return fail(WavError::malformed_riff);
  stream.seekg(0, std::ios::beg);

  std::vector<std::byte> bytes(static_cast<std::size_t>(length));
  if (!stream.read(reinterpret_cast<char*>(bytes.data()), length)) {
    return fail(WavError::truncated);
  }
  if (!tagEquals(bytes.data(), "RIFF") || !tagEquals(bytes.data() + 8, "WAVE")) {
    return fail(WavError::malformed_riff);
  }

  std::uint16_t format = 0;
  std::uint16_t channels = 0;
  std::uint16_t bits = 0;
  std::uint32_t sample_rate = 0;
  const std::byte* audio_data = nullptr;
  std::size_t audio_size = 0;

  std::size_t offset = 12;
  while (offset + 8 <= bytes.size()) {
    const auto chunk_size = static_cast<std::size_t>(readU32(bytes.data() + offset + 4));
    const auto content = offset + 8;
    if (content + chunk_size > bytes.size()) return fail(WavError::truncated);
    if (tagEquals(bytes.data() + offset, "fmt ")) {
      if (chunk_size < 16) return fail(WavError::malformed_riff);
      format = readU16(bytes.data() + content);
      channels = readU16(bytes.data() + content + 2);
      sample_rate = readU32(bytes.data() + content + 4);
      bits = readU16(bytes.data() + content + 14);
    } else if (tagEquals(bytes.data() + offset, "data")) {
      audio_data = bytes.data() + content;
      audio_size = chunk_size;
    }
    offset = content + chunk_size + (chunk_size & 1U);
  }

  if (format == 0 || audio_data == nullptr) return fail(WavError::malformed_riff);
  if (channels != 1) return fail(WavError::unsupported_channels);
  if (!isSupportedSampleRate(static_cast<int>(sample_rate))) {
    return fail(WavError::unsupported_sample_rate);
  }
  if (!((format == 1 && bits == 16) || (format == 3 && bits == 32))) {
    return fail(WavError::unsupported_format);
  }

  const std::size_t bytes_per_sample = bits / 8U;
  if (audio_size % bytes_per_sample != 0) return fail(WavError::truncated);
  AudioBuffer audio{.sample_rate = static_cast<int>(sample_rate)};
  audio.samples.reserve(audio_size / bytes_per_sample);
  for (std::size_t i = 0; i < audio_size; i += bytes_per_sample) {
    if (format == 1) {
      const auto word = readU16(audio_data + i);
      const auto sample = static_cast<std::int16_t>(word);
      audio.samples.push_back(std::max(-1.0F, static_cast<float>(sample) / 32768.0F));
    } else {
      const auto word = readU32(audio_data + i);
      const float sample = std::bit_cast<float>(word);
      if (!std::isfinite(sample)) return fail(WavError::unsupported_format);
      audio.samples.push_back(std::clamp(sample, -1.0F, 1.0F));
    }
  }
  return {.audio = std::move(audio), .error = WavError::none};
}

}  // namespace dubl
