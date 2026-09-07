#include <dubl/wav_reader.hpp>
#include "test_support.hpp"

#include <bit>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

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

std::filesystem::path writePcm16Wav(const std::string& name,
                                    const std::vector<std::int16_t>& samples,
                                    int sample_rate, int channels) {
  auto path = std::filesystem::temp_directory_path() / name;
  std::ofstream stream(path, std::ios::binary | std::ios::trunc);
  const auto data_bytes = static_cast<std::uint32_t>(samples.size() * 2);
  stream.write("RIFF", 4);
  writeU32(stream, 36 + data_bytes);
  stream.write("WAVEfmt ", 8);
  writeU32(stream, 16);
  writeU16(stream, 1);
  writeU16(stream, static_cast<std::uint16_t>(channels));
  writeU32(stream, static_cast<std::uint32_t>(sample_rate));
  writeU32(stream, static_cast<std::uint32_t>(sample_rate * channels * 2));
  writeU16(stream, static_cast<std::uint16_t>(channels * 2));
  writeU16(stream, 16);
  stream.write("data", 4);
  writeU32(stream, data_bytes);
  for (auto sample : samples) writeU16(stream, static_cast<std::uint16_t>(sample));
  return path;
}

std::filesystem::path writeFloat32Wav(const std::string& name, float sample) {
  auto path = std::filesystem::temp_directory_path() / name;
  std::ofstream stream(path, std::ios::binary | std::ios::trunc);
  stream.write("RIFF", 4);
  writeU32(stream, 40);
  stream.write("WAVEfmt ", 8);
  writeU32(stream, 16);
  writeU16(stream, 3);
  writeU16(stream, 1);
  writeU32(stream, 48000);
  writeU32(stream, 192000);
  writeU16(stream, 4);
  writeU16(stream, 32);
  stream.write("data", 4);
  writeU32(stream, 4);
  writeU32(stream, std::bit_cast<std::uint32_t>(sample));
  return path;
}

}  // namespace

int main() {
  const auto valid = writePcm16Wav("dubl-valid.wav", {0, 32767}, 44100, 1);
  const auto stereo = writePcm16Wav("dubl-stereo.wav", {0, 0}, 44100, 2);
  const auto high_rate = writePcm16Wav("dubl-high-rate.wav", {0}, 96000, 1);
  const auto floating = writeFloat32Wav("dubl-float.wav", 0.25F);

  const auto valid_result = dubl::loadMonoWav(valid);
  REQUIRE(valid_result.audio.has_value());
  REQUIRE(valid_result.audio->samples.size() == 2);
  REQUIRE(valid_result.audio->samples[1] > 0.99F);
  REQUIRE(dubl::loadMonoWav(stereo).error == dubl::WavError::unsupported_channels);
  REQUIRE(dubl::loadMonoWav(high_rate).error == dubl::WavError::unsupported_sample_rate);
  const auto float_result = dubl::loadMonoWav(floating);
  REQUIRE(float_result.audio.has_value());
  REQUIRE(float_result.audio->samples.front() == 0.25F);
  REQUIRE(dubl::loadMonoWav("/definitely/not/a/dubl-file.wav").error ==
          dubl::WavError::file_open);

  std::filesystem::remove(valid);
  std::filesystem::remove(stereo);
  std::filesystem::remove(high_rate);
  std::filesystem::remove(floating);
}
