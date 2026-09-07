#include <dubl/audio_buffer.hpp>

#include <cassert>

int main() {
  assert(dubl::isSupportedSampleRate(44100));
  assert(dubl::isSupportedSampleRate(48000));
  assert(!dubl::isSupportedSampleRate(96000));
}
