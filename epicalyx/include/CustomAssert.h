#pragma once

#include "Exceptions.h"


namespace epi::cotyl {

struct AssertionError : Exception {
  AssertionError(std::string&& message) : 
      Exception("Assertion Error", std::move(message)) { }
};

#ifndef NDEBUG
inline void Assert(bool cond, std::string&& message = "") {
  if (!cond) [[unlikely]] {
    throw AssertionError(std::move(message));
  }
}
#else
// make sure namespace in epi::cotyl::Assert is handled properly
#define Assert(cond, ...) _none_call()
inline void _none_call() {  }
#endif

}