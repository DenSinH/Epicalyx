#pragma once

#include <deque>
#include <string>
#include <iostream>

#include "Format.h"


namespace epi::calyx {

template<typename T>
struct Stream {

  bool EOS() {
    return buf.empty() && IsEOS();
  };

  [[nodiscard]] virtual T Get() {
    if (!buf.empty()) {
      T value = buf.front();
      buf.pop_front();
      return value;
    }

    return GetNew();
  }

  void Skip() {
    if (EOS()) {
      throw std::runtime_error("Unexpected EOS");
    }
    [[maybe_unused]] T _ = Get();
  }

  void Skip(int amount) {
    for (int i = 0; i < amount; i++) {
      Skip();
    }
  }

  bool Peek(T& dest, size_t amount) {
    if (EOS()) {
      return false;
    }
    while (buf.size() < amount + 1) {
      if (EOS()) {
        return false;
      }
      buf.push_back(GetNew());
    }
    dest = buf[amount];
    return true;
  }

  bool IsAfter(size_t amount, const T& expect) {
    T value;
    return Peek(value, amount) && value == expect;
  }

  template<typename ...Args>
  bool IsAfter(size_t amount, const T& expect, const Args&... args) {
    return IsAfter(amount, expect) || IsAfter(amount, args...);
  }

  bool SequenceAfter(size_t amount) {
    return true;
  }

  template<typename ...Args>
  bool SequenceAfter(size_t amount, const T& expect, const Args&... args) {
    return IsAfter(amount, expect) && SequenceAfter(amount + 1, args...);
  }

  template<typename Pred>
  bool PredicateAfter(size_t amount, Pred pred) {
    T value;
    return Peek(value, amount) && pred(value);
  }

  template<typename Pred>
  void SkipWhile(Pred pred) {
    while (PredicateAfter(0, pred)) {
      Skip();
    }
  }

  void Expect(const T& expect) {
    const T got = Get();
    if (got != expect) {
      throw FormatExcept("Expected %s, got %s", std::to_string(expect).c_str(), std::to_string(got).c_str());
    }
  }

protected:
  std::deque<T> buf;

  virtual T GetNew() = 0;
  virtual bool IsEOS() = 0;
};

}
