#include "DublAraController.hpp"

#include "DublAraStatus.hpp"

bool DublAraController::doRestoreObjectsFromStream(
    juce::ARAInputStream&, const juce::ARARestoreObjectsFilter*) noexcept {
  return true;
}

bool DublAraController::doStoreObjectsToStream(
    juce::ARAOutputStream&, const juce::ARAStoreObjectsFilter*) noexcept {
  return true;
}

juce::ARAAudioSource* DublAraController::doCreateAudioSource(
    juce::ARADocument* document, ARA::ARAAudioSourceHostRef host_ref) {
  DublAraStatus::observedAudioSource();
  return new juce::ARAAudioSource(document, host_ref);
}

juce::ARAPlaybackRegion* DublAraController::doCreatePlaybackRegion(
    juce::ARAAudioModification* modification,
    ARA::ARAPlaybackRegionHostRef host_ref) {
  DublAraStatus::observedPlaybackRegion();
  return new juce::ARAPlaybackRegion(modification, host_ref);
}

const ARA::ARAFactory* JUCE_CALLTYPE createARAFactory() {
  return juce::ARADocumentControllerSpecialisation::createARAFactory<DublAraController>();
}
