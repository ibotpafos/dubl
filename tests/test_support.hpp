#pragma once

#include <stdexcept>

#define REQUIRE(expression)                                                     \
  do {                                                                          \
    if (!(expression)) throw std::runtime_error("requirement failed: " #expression); \
  } while (false)
