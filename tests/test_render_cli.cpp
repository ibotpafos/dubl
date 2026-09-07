#include <dubl/wav_reader.hpp>
#include "test_support.hpp"

#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <numbers>
#include <string>

namespace {

void writeU16(std::ofstream& stream, std::uint16_t value) {
  stream.put(static_cast<char>(value & 0xff));
  stream.put(static_cast<char>((value >> 8) & 0xff));
}

void writeU32(std::ofstream& stream, std::uint32_t value) {
  for (int shift = 0; shift < 32; shift += 8) stream.put(static_cast<char>((value >> shift) & 0xff));
}

void writeTone(const std::filesystem::path& path, float frequency) {
  constexpr int sample_rate = 48000;
  constexpr int sample_count = sample_rate / 2;
  std::ofstream stream(path, std::ios::binary | std::ios::trunc);
  stream.write("RIFF", 4); writeU32(stream, 36 + sample_count * 2);
  stream.write("WAVEfmt ", 8); writeU32(stream, 16); writeU16(stream, 1);
  writeU16(stream, 1); writeU32(stream, sample_rate); writeU32(stream, sample_rate * 2);
  writeU16(stream, 2); writeU16(stream, 16); stream.write("data", 4);
  writeU32(stream, sample_count * 2);
  for (int i = 0; i < sample_count; ++i) {
    const auto sample = static_cast<std::int16_t>(
        12000.0 * std::sin(2.0 * std::numbers::pi * frequency * i / sample_rate));
    writeU16(stream, static_cast<std::uint16_t>(sample));
  }
}

std::string readAll(const std::filesystem::path& path) {
  std::ifstream stream(path);
  return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

std::string command(const char* executable, const std::filesystem::path& lead,
                    const std::filesystem::path& dub,
                    const std::filesystem::path& output,
                    const std::filesystem::path& report) {
  return std::string{"\""} + executable + "\" --lead \"" + lead.string() +
         "\" --double \"" + dub.string() + "\" --output \"" + output.string() +
         "\" --report \"" + report.string() + "\"";
}

}  // namespace

int main(int argc, char** argv) {
  REQUIRE(argc == 2);
  const auto suffix = std::to_string(
      std::chrono::steady_clock::now().time_since_epoch().count());
  const auto directory = std::filesystem::temp_directory_path() / ("dubl-render-" + suffix);
  std::filesystem::create_directories(directory);
  const auto lead = directory / "lead.wav";
  const auto dub = directory / "double.wav";
  const auto output = directory / "aligned.wav";
  const auto report = directory / "report.json";
  writeTone(lead, 220.0F);
  writeTone(dub, 218.0F);

  const auto invocation = command(argv[1], lead, dub, output, report);
  REQUIRE(std::system(invocation.c_str()) == 0);
  const auto original = dubl::loadMonoWav(dub);
  const auto rendered = dubl::loadMonoWav(output);
  REQUIRE(original.audio.has_value());
  REQUIRE(rendered.audio.has_value());
  REQUIRE(rendered.audio->samples.size() == original.audio->samples.size());
  REQUIRE(readAll(report).find("\"schema_version\": 1") != std::string::npos);
  REQUIRE(std::system(invocation.c_str()) != 0);

  const auto source_overwrite = command(argv[1], lead, dub, dub, directory / "other.json");
  REQUIRE(std::system(source_overwrite.c_str()) != 0);
  std::filesystem::remove_all(directory);
}
