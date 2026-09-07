#include "DublEditor.hpp"

DublEditor::DublEditor(DublProcessor& plugin_owner)
    : AudioProcessorEditor(plugin_owner), owner(plugin_owner) {
  title.setText(juce::CharPointer_UTF8("ДУБЛЬ"), juce::dontSendNotification);
  title.setFont(juce::FontOptions(30.0F, juce::Font::bold));
  title.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(title);
  mode.addItemList({"Natural", "Tight", "Locked"}, 1);
  addAndMakeVisible(mode);
  attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
      owner.state, "mode", mode);
  align.onClick = [this] {
    status.setText("ARA event access is the next milestone", juce::dontSendNotification);
  };
  addAndMakeVisible(align);
  status.setText("Select a mode, then Align", juce::dontSendNotification);
  status.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(status);
  setSize(420, 240);
}

void DublEditor::paint(juce::Graphics& graphics) {
  graphics.fillAll(juce::Colour::fromRGB(19, 20, 24));
  graphics.setColour(juce::Colour::fromRGB(87, 230, 177));
  graphics.drawRoundedRectangle(getLocalBounds().toFloat().reduced(12.0F), 14.0F, 1.5F);
}

void DublEditor::resized() {
  auto area = getLocalBounds().reduced(28);
  title.setBounds(area.removeFromTop(48));
  area.removeFromTop(12);
  mode.setBounds(area.removeFromTop(34).reduced(55, 0));
  area.removeFromTop(18);
  align.setBounds(area.removeFromTop(44).reduced(75, 0));
  area.removeFromTop(12);
  status.setBounds(area.removeFromTop(28));
}
