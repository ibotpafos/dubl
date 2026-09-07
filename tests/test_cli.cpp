#include "test_support.hpp"

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
  for (int shift = 0; shift < 32; shift += 8) {
    stream.put(static_cast<char>((value >> shift) & 0xff));
  }
}

void writeTone(const std::filesystem::path& path, float frequency) {
  constexpr int sample_rate = 48000;
  constexpr int samples = sample_rate / 2;
  std::ofstream stream(path, std::ios::binary | std::ios::trunc);
  stream.write("RIFF", 4);
  writeU32(stream, 36 + samples * 2);
  stream.write("WAVEfmt ", 8);
  writeU32(stream, 16);
  writeU16(stream, 1);
  writeU16(stream, 1);
  writeU32(stream, sample_rate);
  writeU32(stream, sample_rate * 2);
  writeU16(stream, 2);
  writeU16(stream, 16);
  stream.write("data", 4);
  writeU32(stream, samples * 2);
  for (int i = 0; i < samples; ++i) {
    const auto value = static_cast<std::int16_t>(
        12000.0 * std::sin(2.0 * std::numbers::pi * frequency * i / sample_rate));
    writeU16(stream, static_cast<std::uint16_t>(value));
  }
}

std::string readAll(const std::filesystem::path& path) {
  std::ifstream stream(path);
  return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
}

}  // namespace

int main(int argc, char** argv) {
  REQUIRE(argc == 2);
  const auto directory = std::filesystem::temp_directory_path() / "dubl-cli-test";
  std::filesystem::create_directories(directory);
  const auto lead = directory / "lead.wav";
  const auto dub = directory / "double.wav";
  const auto report = directory / "report.json";
  writeTone(lead, 220.0F);
  writeTone(dub, 218.0F);

  const std::string command = std::string{"\""} + argv[1] + "\" --lead \"" +
      lead.string() + "\" --double \"" + dub.string() + "\" --report \"" +
      report.string() + "\"";
  REQUIRE(std::system(command.c_str()) == 0);
  const auto json = readAll(report);
  REQUIRE(json.find("\"schema_version\": 1") != std::string::npos);
  REQUIRE(json.find("\"confidence\"") != std::string::npos);
  REQUIRE(json.find("\"warp_points\"") != std::string::npos);
  REQUIRE(json.find("\"input\"") != std::string::npos);
  REQUIRE(json.find(directory.string()) == std::string::npos);

  std::filesystem::remove_all(directory);
}
