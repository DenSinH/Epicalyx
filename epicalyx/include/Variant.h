#pragma once

#include <type_traits>
#include <stdexcept>
#include "TypeTraits.h"
#include "swl/variant.hpp"

namespace epi::cotyl {


template<typename Parent, typename... Ts>
requires (std::is_base_of_v<Parent, Ts> && ...)
struct Variant {
  template<typename T>
  requires (std::is_same_v<T, Ts> || ...)
  static constexpr std::size_t type_index_v = (std::size_t)swl::variant<Ts...>::template index_of<T>;

  // to speed up compilation time for large variants
  ~Variant();
  Variant(const Variant& other) = default;
  Variant(Variant&& other) = default;

  template<typename T>
  requires (std::is_same_v<T, Ts> || ...)
  Variant(T&& value) : value{std::move(value)} { }

  template<typename T>
  requires (std::is_same_v<T, Ts> || ...)
  Variant(const T& value) : value{value} { }

  template<typename T, typename... Args>
  requires (std::is_same_v<T, Ts> || ...)
  void emplace(Args&&... args) {
    value.template emplace<T>(std::forward<Args...>(args)...);
  }

  template<typename T>
  bool holds_alternative() const {
    return swl::holds_alternative<T>(value);
  }

  std::size_t index() const {
    return value.index();
  }

  Parent* operator->() {
    return swl::visit([](auto& arg) -> Parent* { return &arg; }, value);
  }

  const Parent* operator->() const {
    return swl::visit([](const auto& arg) -> const Parent* { return &arg; }, value);
  }

  Parent& operator*() { 
    return swl::visit([](auto& arg) -> Parent& { return arg; }, value);
  }

  const Parent& operator*() const { 
    return swl::visit([](const auto& arg) -> const Parent& { return arg; }, value);
  }

  template<typename T>
  requires (std::is_same_v<T, Ts> || ...)
  T& get() {
#ifndef NDEBUG
    return swl::get<T>(value);
#else 
    return swl::unsafe_get<T>(value);
#endif
  }

  template<typename T>
  requires (std::is_same_v<T, Ts> || ...)
  const T& get() const {
#ifndef NDEBUG
    return swl::get<T>(value);
#else 
    return swl::unsafe_get<T>(value);
#endif
  }
  
  template<typename R, typename... Args>
  R visit(Args&&... args) {
    return swl::visit(swl::overloaded{std::forward<Args>(args)...}, value);
  }

  template<typename R, typename... Args>
  R visit(Args&&... args) const {
    return swl::visit(swl::overloaded{std::forward<Args>(args)...}, value);
  }

protected:
  
  swl::variant<Ts...> value;
};

}