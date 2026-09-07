#pragma once

#include "DublProcessor.hpp"

#include <juce_gui_basics/juce_gui_basics.h>

class DublEditor final : public juce::AudioProcessorEditor {
 public:
  explicit DublEditor(DublProcessor&);
  void paint(juce::Graphics&) override;
  void resized() override;

 private:
  DublProcessor& owner;
  juce::Label title;
  juce::ComboBox mode;
  juce::TextButton align{"ALIGN"};
  juce::Label status;
  std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
};
