cmake_minimum_required(VERSION 3.25)
project(dubl VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(dubl_core STATIC src/wav_reader.cpp src/wav_writer.cpp src/features.cpp src/alignment.cpp src/warp_plan.cpp)
target_include_directories(dubl_core PUBLIC include)

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

add_executable(dubl_analyse apps/dubl_analyse.cpp)
target_link_libraries(dubl_analyse PRIVATE dubl_core)

add_executable(dubl_test_cli tests/test_cli.cpp)
target_link_libraries(dubl_test_cli PRIVATE dubl_core)
add_test(NAME dubl_test_cli COMMAND dubl_test_cli $<TARGET_FILE:dubl_analyse>)
