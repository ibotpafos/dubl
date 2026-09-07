#include "DublProcessor.hpp"

#include "DublEditor.hpp"

DublProcessor::DublProcessor()
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "DUBLState", parameters()) {}

juce::AudioProcessorValueTreeState::ParameterLayout DublProcessor::parameters() {
  juce::AudioProcessorValueTreeState::ParameterLayout layout;
  layout.add(std::make_unique<juce::AudioParameterChoice>(
      juce::ParameterID{"mode", 1}, "Mode",
      juce::StringArray{"Natural", "Tight", "Locked"}, 0));
  return layout;
}

bool DublProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const {
  const auto input = layouts.getMainInputChannelSet();
  return input == layouts.getMainOutputChannelSet() &&
         (input == juce::AudioChannelSet::mono() ||
          input == juce::AudioChannelSet::stereo());
}

void DublProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                 juce::MidiBuffer&) {
  juce::ScopedNoDenormals no_denormals;
  for (int channel = getTotalNumInputChannels();
       channel < getTotalNumOutputChannels(); ++channel) {
    buffer.clear(channel, 0, buffer.getNumSamples());
  }
}

juce::AudioProcessorEditor* DublProcessor::createEditor() {
  return new DublEditor(*this);
}

void DublProcessor::getStateInformation(juce::MemoryBlock& destination) {
  if (const auto xml = state.copyState().createXml()) copyXmlToBinary(*xml, destination);
}

void DublProcessor::setStateInformation(const void* data, const int size) {
  if (const auto xml = getXmlFromBinary(data, size)) {
    if (xml->hasTagName(state.state.getType())) state.replaceState(juce::ValueTree::fromXml(*xml));
  }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
  return new DublProcessor();
}
