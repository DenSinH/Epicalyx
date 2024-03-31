#pragma once

#include <string>
#include <stdexcept>


namespace epi::cotyl {

#ifndef NDEBUG
static void Assert(bool cond, const std::string& message = "") {
  if (!cond) [[unlikely]] {
    throw std::runtime_error("Assertion failed: " + message);
  }
}
#else
// make sure namespace in epi::cotyl::Assert is handled properly
#define Assert(cond, ...) _none_call()
static void _none_call() {  }
#endif

}