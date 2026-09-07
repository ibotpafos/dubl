#include <dubl/audio_buffer.hpp>
#include "test_support.hpp"

int main() {
  REQUIRE(dubl::isSupportedSampleRate(44100));
  REQUIRE(dubl::isSupportedSampleRate(48000));
  REQUIRE(!dubl::isSupportedSampleRate(96000));
}
