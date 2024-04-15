#pragma once

#include <type_traits>
#include <stdexcept>
#include "TypeTraits.h"

namespace epi::cotyl {


template<typename Parent, typename... Ts>
requires (std::is_base_of_v<Parent, Ts> && ...)
struct Variant {

  template<typename T>
  requires (std::is_same_v<T, Ts> || ...)
  static constexpr std::size_t type_index_v = cotyl::type_index_v<T, Ts...>;

  Variant() = delete;

  Variant(Variant<Parent, Ts...>&& other) :
      type_index{other.type_index} {
    other.visit<void>([&](auto&& o) {
      using type_t = std::decay_t<decltype(o)>;
      new(&memory) type_t{std::move(o)};
    });
  }

  Variant(const Variant<Parent, Ts...>& other) :
      type_index{other.type_index} {
    other.visit<void>([&](const auto& o) {
      using type_t = std::decay_t<decltype(o)>;
      new(&memory) type_t{o};
    });
  }

  ~Variant() {
    visit<void>([&](auto&& o) {
      using type_t = std::decay_t<decltype(o)>;
      reinterpret_cast<type_t*>(&memory)->~type_t();
    });
  }

//   Variant<Parent, Ts...>& operator=(const Variant<Parent, Ts...>& other) {
//     type_index = other.type_index;
//     other.visit<void>([&](const auto& o) {
//       using type_t = std::decay_t<decltype(o)>;
//       *(reinterpret_cast<type_t*>(&memory)) = o;
//     });
//     return *this;
//   }

  template<typename T>
  requires (std::is_same_v<T, Ts> || ...)
  [[clang::always_inline]] Variant(T&& value) : type_index{type_index_v<T>} {
    new(&memory) T{std::move(value)};
  }

  template<typename T>
  requires (std::is_same_v<T, Ts> || ...)
  [[clang::always_inline]] Variant(const T& value) : type_index{type_index_v<T>} {
    new(&memory) T{value};
  }

  template<typename T, typename... Args>
  requires (std::is_same_v<T, Ts> || ...)
  [[clang::always_inline]] void emplace(Args&&... args) {
    new(&memory) T(std::forward<Args...>(args)...);
    type_index = type_index_v<T>;
  }

  template<typename T>
  requires (std::is_same_v<T, Ts> || ...)
  [[clang::always_inline]] bool holds_alternative() const {
    return type_index == type_index_v<T>;
  }

  [[clang::always_inline]] std::size_t index() const {
    return type_index;
  }

  [[clang::always_inline]] Parent* operator->() {
    return reinterpret_cast<Parent*>(&memory);
  }

  [[clang::always_inline]] const Parent* operator->() const {
    return reinterpret_cast<const Parent*>(&memory);
  }

  [[clang::always_inline]] Parent& operator*() { 
    return *(reinterpret_cast<Parent*>(&memory));
  }

  [[clang::always_inline]] const Parent& operator*() const { 
    return *(reinterpret_cast<const Parent*>(&memory));
  }

  template<typename T>
  requires (std::is_same_v<T, Ts> || ...)
  [[clang::always_inline]] T& get() {
    return *(reinterpret_cast<T*>(&memory));
  }

  template<typename T>
  requires (std::is_same_v<T, Ts> || ...)
  [[clang::always_inline]] const T& get() const {
    return *(reinterpret_cast<const T*>(&memory));
  }
  
  template<typename R, typename... Args>
  R visit(Args&&... args) {
    const auto& callable = overloaded{args...};
    if constexpr(std::is_same_v<R, void>) {
        return [&]() -> void {
            ((holds_alternative<Ts>() && (void(callable(get<Ts>())), 1)) || ...)
                || (void(throw_exhausted()), 0);
        }();
    }
    else {
        return [&]() -> R {
            R ret;
            ((holds_alternative<Ts>() && (void(ret = callable(get<Ts>())), 1)) || ...)
                || (void(throw_exhausted()), 0);
            return ret;
        }();
    }
  }

  template<typename R, typename... Args>
  R visit(Args&&... args) const {
    const auto& callable = overloaded{args...};
    if constexpr(std::is_same_v<R, void>) {
        return [&]() -> void {
            ((holds_alternative<Ts>() && (void(callable(get<Ts>())), 1)) || ...)
                || (void(throw_exhausted()), 0);
        }();
    }
    else {
        return [&]() -> R {
            R ret;
            ((holds_alternative<Ts>() && (void(ret = callable(get<Ts>())), 1)) || ...)
                || (void(throw_exhausted()), 0);
            return ret;
        }();
    }
  }

private:

  [[noreturn]] void throw_exhausted() const {
    throw std::runtime_error("Exhausted variant visit");
  }

  // for simple visitor pattern
  template<class... Args>
  struct overloaded : Args... { using Args::operator()...; };
  
  int type_index;
  std::aligned_union_t<0, Ts...> memory;
};

}