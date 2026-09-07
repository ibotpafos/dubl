#include "DublProcessor.hpp"
#include "DublAraStatus.hpp"
#include "test_support.hpp"

#include <cmath>

int main() {
  const auto sources_before = DublAraStatus::audioSourcesObserved();
  DublAraStatus::observedAudioSource();
  REQUIRE(DublAraStatus::audioSourcesObserved() == sources_before + 1);
  DublProcessor processor;
  processor.prepareToPlay(48000.0, 128);
  juce::AudioBuffer<float> audio(2, 128);
  for (int channel = 0; channel < audio.getNumChannels(); ++channel) {
    for (int sample = 0; sample < audio.getNumSamples(); ++sample) {
      audio.setSample(channel, sample, static_cast<float>(sample) / 128.0F);
    }
  }
  const auto before = audio.getSample(1, 63);
  juce::MidiBuffer midi;
  processor.processBlock(audio, midi);
  REQUIRE(std::abs(audio.getSample(1, 63) - before) < 0.000001F);

  auto* mode = processor.state.getParameter("mode");
  REQUIRE(mode != nullptr);
  mode->setValueNotifyingHost(1.0F);
  juce::MemoryBlock saved;
  processor.getStateInformation(saved);
  mode->setValueNotifyingHost(0.0F);
  processor.setStateInformation(saved.getData(), static_cast<int>(saved.getSize()));
  REQUIRE(mode->getValue() > 0.99F);
}
