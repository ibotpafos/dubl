#include <dubl/wav_reader.hpp>
#include <dubl/wav_writer.hpp>
#include "test_support.hpp"

#include <chrono>
#include <cmath>
#include <filesystem>
#include <limits>
#include <string>

int main() {
  const auto suffix = std::to_string(
      std::chrono::steady_clock::now().time_since_epoch().count());
  const auto directory = std::filesystem::temp_directory_path() / ("dubl-writer-" + suffix);
  std::filesystem::create_directories(directory);
  const auto output = directory / "result.wav";

  const dubl::AudioBuffer audio{
      .sample_rate = 48000,
      .samples = {0.0F, 0.5F, -0.5F, 1.0F, -1.0F},
  };
  REQUIRE(dubl::writeMonoPcm16Wav(output, audio).error == dubl::WavWriteError::none);
  const auto loaded = dubl::loadMonoWav(output);
  REQUIRE(loaded.audio.has_value());
  REQUIRE(loaded.audio->sample_rate == 48000);
  REQUIRE(loaded.audio->samples.size() == audio.samples.size());
  REQUIRE(std::abs(loaded.audio->samples[1] - 0.5F) < 1.0F / 32768.0F);
  REQUIRE(dubl::writeMonoPcm16Wav(output, audio).error ==
          dubl::WavWriteError::destination_exists);

  const auto invalid_output = directory / "invalid.wav";
  auto invalid = audio;
  invalid.samples[0] = std::numeric_limits<float>::quiet_NaN();
  REQUIRE(dubl::writeMonoPcm16Wav(invalid_output, invalid).error ==
          dubl::WavWriteError::invalid_audio);
  REQUIRE(!std::filesystem::exists(invalid_output));

  std::filesystem::remove_all(directory);
}
