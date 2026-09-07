cmake_minimum_required(VERSION 3.25)
project(dubl VERSION 0.1.0 LANGUAGES C CXX)

include(FetchContent)
FetchContent_Declare(
  signalsmith_stretch
  GIT_REPOSITORY https://github.com/Signalsmith-Audio/signalsmith-stretch.git
  GIT_TAG 57b93f4e9206a089a45387eaa39bdc9f310d3308
  GIT_SHALLOW OFF
)
FetchContent_MakeAvailable(signalsmith_stretch)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(dubl_core STATIC src/wav_reader.cpp src/wav_writer.cpp src/features.cpp src/alignment.cpp src/warp_plan.cpp src/time_renderer.cpp src/pitch_decision.cpp)
target_include_directories(dubl_core PUBLIC include)

add_library(dubl_rendering STATIC src/pitch_renderer.cpp)
target_include_directories(dubl_rendering PUBLIC include)
target_link_libraries(dubl_rendering PUBLIC dubl_core PRIVATE signalsmith-stretch)

add_executable(dubl_test_audio_buffer tests/test_audio_buffer.cpp)
target_link_libraries(dubl_test_audio_buffer PRIVATE dubl_core)
add_test(NAME dubl_test_audio_buffer COMMAND dubl_test_audio_buffer)

add_executable(dubl_test_wav_reader tests/test_wav_reader.cpp)
target_link_libraries(dubl_test_wav_reader PRIVATE dubl_core)
add_test(NAME dubl_test_wav_reader COMMAND dubl_test_wav_reader)

add_executable(dubl_test_wav_writer tests/test_wav_writer.cpp)
target_link_libraries(dubl_test_wav_writer PRIVATE dubl_core)
add_test(NAME dubl_test_wav_writer COMMAND dubl_test_wav_writer)

add_executable(dubl_test_features tests/test_features.cpp)
target_link_libraries(dubl_test_features PRIVATE dubl_core)
add_test(NAME dubl_test_features COMMAND dubl_test_features)

add_executable(dubl_test_alignment tests/test_alignment.cpp)
target_link_libraries(dubl_test_alignment PRIVATE dubl_core)
add_test(NAME dubl_test_alignment COMMAND dubl_test_alignment)

add_executable(dubl_test_warp_plan tests/test_warp_plan.cpp)
target_link_libraries(dubl_test_warp_plan PRIVATE dubl_core)
add_test(NAME dubl_test_warp_plan COMMAND dubl_test_warp_plan)

add_executable(dubl_test_time_renderer tests/test_time_renderer.cpp)
target_link_libraries(dubl_test_time_renderer PRIVATE dubl_core)
add_test(NAME dubl_test_time_renderer COMMAND dubl_test_time_renderer)

add_executable(dubl_test_pitch_decision tests/test_pitch_decision.cpp)
target_link_libraries(dubl_test_pitch_decision PRIVATE dubl_core)
add_test(NAME dubl_test_pitch_decision COMMAND dubl_test_pitch_decision)

add_executable(dubl_test_pitch_renderer tests/test_pitch_renderer.cpp)
target_link_libraries(dubl_test_pitch_renderer PRIVATE dubl_rendering)
add_test(NAME dubl_test_pitch_renderer COMMAND dubl_test_pitch_renderer)

add_executable(dubl_analyse apps/dubl_analyse.cpp)
target_link_libraries(dubl_analyse PRIVATE dubl_core)

add_executable(dubl_test_cli tests/test_cli.cpp)
target_link_libraries(dubl_test_cli PRIVATE dubl_core)
add_test(NAME dubl_test_cli COMMAND dubl_test_cli $<TARGET_FILE:dubl_analyse>)

add_executable(dubl_render apps/dubl_render.cpp)
target_link_libraries(dubl_render PRIVATE dubl_rendering)

add_executable(dubl_test_render_cli tests/test_render_cli.cpp)
target_link_libraries(dubl_test_render_cli PRIVATE dubl_core)
add_test(NAME dubl_test_render_cli COMMAND dubl_test_render_cli $<TARGET_FILE:dubl_render>)

option(DUBL_BUILD_PLUGIN "Build the DUBL VST3 plug-in" OFF)
if(DUBL_BUILD_PLUGIN)
  FetchContent_Declare(
    juce
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 2cdfca8feb300fb424002ba2c2751569e5bacb64
    GIT_SHALLOW OFF
  )
  FetchContent_MakeAvailable(juce)

  FetchContent_Declare(
    ara_sdk
    GIT_REPOSITORY https://github.com/Celemony/ARA_SDK.git
    GIT_TAG 0005c532bb03eb210e7bb490329fc6d7ba1792c7
    GIT_SHALLOW OFF
    GIT_SUBMODULES_RECURSE TRUE
  )
  FetchContent_MakeAvailable(ara_sdk)
  juce_set_ara_sdk_path(${ara_sdk_SOURCE_DIR})

  juce_add_plugin(DUBL
    COMPANY_NAME "DUBL Open Source"
    BUNDLE_ID "org.dubl.audio.plugin"
    PLUGIN_MANUFACTURER_CODE Dubl
    PLUGIN_CODE Dbl1
    FORMATS VST3
    PRODUCT_NAME "DUBL"
    IS_ARA_EFFECT TRUE
    ARA_FACTORY_ID "\"org.dubl.audio.factory.1\""
    ARA_DOCUMENT_ARCHIVE_ID "\"org.dubl.audio.archive.1\""
    COPY_PLUGIN_AFTER_BUILD FALSE
  )
  target_sources(DUBL PRIVATE plugin/DublProcessor.cpp plugin/DublProcessor.hpp
                              plugin/DublEditor.cpp plugin/DublEditor.hpp
                              plugin/DublAraStatus.cpp plugin/DublAraStatus.hpp
                              plugin/DublAraController.cpp plugin/DublAraController.hpp)
  target_compile_definitions(DUBL PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
                                         JUCE_VST3_CAN_REPLACE_VST2=0)
  target_link_libraries(DUBL PRIVATE dubl_rendering juce::juce_audio_utils
                                     juce::juce_gui_extra
                              PUBLIC juce::juce_recommended_config_flags
                                     juce::juce_recommended_lto_flags
                                     juce::juce_recommended_warning_flags)
  if(APPLE)
    add_custom_command(TARGET DUBL_VST3 POST_BUILD
      COMMAND codesign --force --deep --sign - "$<TARGET_BUNDLE_DIR:DUBL_VST3>"
      COMMENT "Applying final ad-hoc signature after VST3 manifest generation")
  endif()

  add_executable(dubl_test_plugin tests/test_plugin_processor.cpp
                                  plugin/DublProcessor.cpp
                                  plugin/DublProcessor.hpp
                                  plugin/DublEditor.cpp
                                  plugin/DublEditor.hpp
                                  plugin/DublAraStatus.cpp
                                  plugin/DublAraStatus.hpp)
  target_include_directories(dubl_test_plugin PRIVATE plugin tests)
  target_compile_definitions(dubl_test_plugin PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
                                                      JUCE_VST3_CAN_REPLACE_VST2=0)
  target_link_libraries(dubl_test_plugin PRIVATE dubl_rendering juce::juce_audio_utils
                         juce::juce_gui_extra juce::juce_recommended_config_flags
                         juce::juce_recommended_warning_flags)
  add_test(NAME dubl_test_plugin COMMAND dubl_test_plugin)
endif()
