cmake_minimum_required(VERSION 3.25)
project(dubl VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(dubl_core INTERFACE)
target_include_directories(dubl_core INTERFACE include)

add_executable(dubl_test_audio_buffer tests/test_audio_buffer.cpp)
target_link_libraries(dubl_test_audio_buffer PRIVATE dubl_core)
add_test(NAME dubl_test_audio_buffer COMMAND dubl_test_audio_buffer)
