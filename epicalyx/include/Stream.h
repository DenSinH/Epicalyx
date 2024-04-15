#pragma once

#include "Format.h"
#include "Variant.h"
#include "Exceptions.h"
#include "Locatable.h"

#include <deque>
#include <string>
#include <iostream>
#include <concepts>


namespace epi::cotyl {

namespace detail {

template<typename T>
struct base_type;

template<typename T>
concept has_base_t = requires {
    typename T::base_t;
    std::is_base_of<typename T::base_t, T>::value;
};

template<has_base_t T>
struct base_type<T> : base_type<typename T::base_t> { };

template<typename T>
struct base_type<std::unique_ptr<T>> {
  using type = T;
};

template<typename T>
struct base_type<std::shared_ptr<T>> {
  using type = T;
};

template<typename T, typename... Args>
struct base_type<cotyl::Variant<T, Args...>> {
  using type = T;
};

template<typename T>
struct base_type {
  using type = std::decay_t<T>;
};

template<typename T, typename U>
concept Comparable = requires(T t, U u) {
    { t == u } -> std::same_as<bool>;
};

}

template<typename T>
struct Stream : public Locatable {

  using base_t = typename detail::base_type<T>::type;
  static constexpr bool deref = !std::is_same_v<base_t, T>;

  bool EOS() {
    return buf.empty() && IsEOS();
  };

  [[nodiscard]] T Get() {
    if (!buf.empty()) {
      T value = std::move(buf.front());
      buf.pop_front();
      return std::move(value);
    }

    return GetNew();
  }

  void Skip() {
    if (EOS()) {
      throw cotyl::EndOfFileException();
    }
    [[maybe_unused]] T _ = Get();
  }

  void Skip(int amount) {
    for (int i = 0; i < amount; i++) {
      Skip();
    }
  }

  bool Peek(base_t& dest, size_t amount = 0) {
    const base_t* d;
    if (!Peek(d, amount)) {
      return false;
    }
    dest = *d;
    return true;
  }

  bool Peek(const base_t*& dest, size_t amount = 0) {
    if (EOS()) {
      return false;
    }
    while (buf.size() < amount + 1) {
      if (IsEOS()) {
        return false;
      }
      buf.push_back(GetNew());
    }
    if constexpr(deref) {
      dest = &(*buf[amount]);
    }
    else {
      dest = &buf[amount];
    }
    return true;
  }

  const base_t* ForcePeek(size_t amount = 0) {
    const base_t* value;
    if (!Peek(value, amount)) {
      throw cotyl::EndOfFileException();
    }
    return value;
  }

  template<typename S>
  requires detail::Comparable<base_t, S>
  bool IsAfter(size_t amount, const S& expect) {
    const base_t* value;
    return Peek(value, amount) && *value == expect;
  }

  template<typename S, typename ...Args>
  requires (detail::Comparable<base_t, S> && (detail::Comparable<base_t, Args> && ...))
  bool IsAfter(size_t amount, const S& expect, const Args& ... args) {
    return IsAfter(amount, expect) || IsAfter(amount, args...);
  }

  template<typename S>
  requires detail::Comparable<base_t, S>
  T Expect(const S& expect) {
    if (!IsAfter(0, expect)) {
      throw cotyl::FormatExceptStr("Invalid token: expected '%s', got '%s'", expect, Get());
    }
    return Get();
  }

  bool SequenceAfter(size_t amount) {
    return true;
  }

  template<typename S, typename ...Args>
  requires (detail::Comparable<base_t, S> && (detail::Comparable<base_t, Args> && ...))
  bool SequenceAfter(size_t amount, const S& expect, const Args& ... args) {
    return IsAfter(amount, expect) && SequenceAfter(amount + 1, args...);
  }

  template<typename S>
  requires detail::Comparable<base_t, S>
  void EatSequence(const S& expect) {
    Eat(expect);
  }

  template<typename S, typename ...Args>
  requires (detail::Comparable<base_t, S> && (detail::Comparable<base_t, Args> && ...))
  void EatSequence(const S& expect, const Args&... args) {
    Eat(expect);
    EatSequence(args...);
  }

  template<typename Pred>
  bool PredicateAfter(size_t amount, Pred pred) {
    const base_t* value;
    return Peek(value, amount) && pred(*value);
  }

  template<typename Pred>
  void SkipWhile(Pred pred) {
    while (PredicateAfter(0, pred)) {
      Skip();
    }
  }

  template<typename S>
  requires detail::Comparable<base_t, S>
  T Eat(const S& expect) {
    T got = Get();
    if constexpr(deref) {
      if (!(*got == expect)) {
        throw FormatExceptStr("Invalid token: expected '%s', got '%s'", expect, got);
      }
    }
    else {
      if (!(got == expect)) {
        throw FormatExceptStr("Invalid token: expected '%s', got '%s'", expect, got);
      }
    }
    return std::move(got);
  }

  template<typename S>
  requires detail::Comparable<base_t, S>
  bool EatIf(const S& expect) {
    if (IsAfter(0, expect)) {
      Skip();
      return true;
    }
    return false;
  }

protected:
  size_t BufSize() const { return buf.size(); }
  virtual T GetNew() = 0;
  virtual bool IsEOS() = 0;

private:
  // only for internal EOS control
  std::deque<T> buf;
};

}