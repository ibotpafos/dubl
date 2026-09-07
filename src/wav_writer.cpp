#include <dubl/wav_writer.hpp>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <limits>
#include <string>

namespace dubl {
namespace {

void writeU16(std::ofstream& stream, std::uint16_t value) {
  stream.put(static_cast<char>(value & 0xff));
  stream.put(static_cast<char>((value >> 8) & 0xff));
}

void writeU32(std::ofstream& stream, std::uint32_t value) {
  for (int shift = 0; shift < 32; shift += 8) {
    stream.put(static_cast<char>((value >> shift) & 0xff));
  }
}

std::filesystem::path temporaryPathFor(const std::filesystem::path& path) {
  static std::atomic<std::uint64_t> counter{0};
  const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
  return path.parent_path() /
         ("." + path.filename().string() + ".dubl-tmp-" + std::to_string(stamp) +
          "-" + std::to_string(counter.fetch_add(1)));
}

}  // namespace

WavWriteResult writeMonoPcm16Wav(const std::filesystem::path& path,
                                 const AudioBuffer& audio) {
  std::error_code error;
  if (std::filesystem::exists(path, error)) {
    return {.error = WavWriteError::destination_exists};
  }
  if (error || !isSupportedSampleRate(audio.sample_rate) || audio.samples.empty() ||
      audio.samples.size() > (std::numeric_limits<std::uint32_t>::max() - 36U) / 2U) {
    return {.error = WavWriteError::invalid_audio};
  }
  for (const float sample : audio.samples) {
    if (!std::isfinite(sample)) return {.error = WavWriteError::invalid_audio};
  }

  const auto temporary = temporaryPathFor(path);
  std::ofstream stream(temporary, std::ios::binary | std::ios::trunc);
  if (!stream) return {.error = WavWriteError::open_failed};
  const auto data_bytes = static_cast<std::uint32_t>(audio.samples.size() * 2U);
  stream.write("RIFF", 4);
  writeU32(stream, 36U + data_bytes);
  stream.write("WAVEfmt ", 8);
  writeU32(stream, 16);
  writeU16(stream, 1);
  writeU16(stream, 1);
  writeU32(stream, static_cast<std::uint32_t>(audio.sample_rate));
  writeU32(stream, static_cast<std::uint32_t>(audio.sample_rate * 2));
  writeU16(stream, 2);
  writeU16(stream, 16);
  stream.write("data", 4);
  writeU32(stream, data_bytes);
  for (const float sample : audio.samples) {
    const auto pcm = static_cast<std::int16_t>(
        std::lround(std::clamp(sample, -1.0F, 1.0F) * 32767.0F));
    writeU16(stream, static_cast<std::uint16_t>(pcm));
  }
  stream.flush();
  if (!stream) {
    stream.close();
    std::filesystem::remove(temporary, error);
    return {.error = WavWriteError::write_failed};
  }
  stream.close();

  std::filesystem::create_hard_link(temporary, path, error);
  if (error) {
    const bool destination_exists = std::filesystem::exists(path);
    std::filesystem::remove(temporary, error);
    return {.error = destination_exists ? WavWriteError::destination_exists
                                        : WavWriteError::publish_failed};
  }
  std::filesystem::remove(temporary, error);
  return {.error = WavWriteError::none};
}

}  // namespace dubl
