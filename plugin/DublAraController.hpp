#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

class DublAraController final : public juce::ARADocumentControllerSpecialisation {
 public:
  using ARADocumentControllerSpecialisation::ARADocumentControllerSpecialisation;

 protected:
  bool doRestoreObjectsFromStream(juce::ARAInputStream&,
                                  const juce::ARARestoreObjectsFilter*) noexcept override;
  bool doStoreObjectsToStream(juce::ARAOutputStream&,
                              const juce::ARAStoreObjectsFilter*) noexcept override;
  juce::ARAAudioSource* doCreateAudioSource(
      juce::ARADocument*, ARA::ARAAudioSourceHostRef) override;
  juce::ARAPlaybackRegion* doCreatePlaybackRegion(
      juce::ARAAudioModification*, ARA::ARAPlaybackRegionHostRef) override;
};
