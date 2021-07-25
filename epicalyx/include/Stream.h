#pragma once

#include "Format.h"

#include <deque>
#include <string>
#include <iostream>


namespace epi::cotyl {

struct Locatable {
  virtual void PrintLoc() = 0;
};

template<typename T>
struct Stream {

  bool EOS() {
    return buf.empty() && IsEOS();
  };

  [[nodiscard]] T Get() {
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

  bool Peek(T& dest, size_t amount = 0) {
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

  T ForcePeek(size_t amount = 0) {
    T value;
    if (!Peek(value, amount)) {
      throw std::runtime_error("Unexpected end of stream");
    }
    return value;
  }

  virtual bool IsAfter(size_t amount, const T& expect) {
    T value;
    return Peek(value, amount) && value == expect;
  }

  template<typename ...Args>
  bool IsAfter(size_t amount, const T& expect, const Args& ... args) {
    return IsAfter(amount, expect) || IsAfter(amount, args...);
  }

  void Expect(const T& expect) {
    if (!IsAfter(0, expect)) {
      throw cotyl::FormatExceptStr("Invalid token: expected '%s', got '%s'", expect, Get());
    }
  }

  bool SequenceAfter(size_t amount) {
    return true;
  }

  template<typename ...Args>
  bool SequenceAfter(size_t amount, const T& expect, const Args& ... args) {
    return IsAfter(amount, expect) && SequenceAfter(amount + 1, args...);
  }

  void EatSequence(const T& expect) {
    Eat(expect);
  }

  template<typename ...Args>
  void EatSequence(const T& expect, const Args&... args) {
    Eat(expect);
    EatSequence(args...);
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

  virtual T Eat(const T& expect) {
    using std::to_string;

    const T got = Get();
    if (got != expect) {
      throw FormatExceptStr("Invalid token: expected '%s', got '%s'", expect, got);
    }
    return got;
  }

protected:
  std::deque<T> buf;

  virtual T GetNew() = 0;

  virtual bool IsEOS() = 0;
};


template<typename T>
struct pStream : public Stream<std::shared_ptr<T>> {

  bool IsAfter(size_t amount, const T& expect) {
    std::shared_ptr<T> value;
    return Stream<std::shared_ptr<T>>::Peek(value, amount) && *value == expect;
  }

  bool IsAfter(size_t amount, const std::shared_ptr<T>& expect) override {
    return IsAfter(amount, *expect);
  }

  template<typename ...Args>
  bool IsAfter(size_t amount, const T& expect, const Args&... args) {
    return IsAfter(amount, expect) || IsAfter(amount, args...);
  }

  void Expect(const T& expect) {
    if (!IsAfter(0, expect)) {
      throw cotyl::FormatExceptStr("Invalid token: expected '%s', got '%s'", expect, Stream<std::shared_ptr<T>>::Get());
    }
  }

  std::shared_ptr<T> Eat(const T& expect) {
    using std::to_string;

    const auto got = Stream<std::shared_ptr<T>>::Get();
    if (!(*got == expect)) {
      throw FormatExceptStr("Invalid token: expected '%s', got '%s'", expect, got);
    }
    return got;
  }

  bool EatIf(const T& expect) {
    if (IsAfter(0, expect)) {
      Stream<std::shared_ptr<T>>::Skip();
      return true;
    }
    return false;
  }

  std::shared_ptr<T> Eat(const std::shared_ptr<T>& expect) override {
    return Eat(*expect);
  }

  void EatSequence(const T& expect) {
    Eat(expect);
  }

  template<typename ...Args>
  void EatSequence(const T& expect, const Args&... args) {
    Eat(expect);
    EatSequence(args...);
  }

};

}