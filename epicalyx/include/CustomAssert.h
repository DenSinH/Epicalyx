#pragma once

#include <string>
#include <stdexcept>


namespace epi::cotyl {

static void Assert(bool cond, const std::string& message = "") {
#ifndef NDEBUG
  if (!cond) [[unlikely]] {
    throw std::runtime_error("Assertion failed: " + message);
  }
#endif
}

}