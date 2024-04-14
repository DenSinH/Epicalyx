#pragma once

#include <variant>
#include <type_traits>

#include "Is.h"


namespace epi::cotyl {

template<typename Parent, typename... Ts>
requires (std::is_base_of_v<Parent, Ts> && ...)
struct Variant {

  template<typename T>
  Variant(T&& value) : value{std::move(value)} { }

  template<typename T>
  Variant(const T& value) : value{value} { }

  template<typename T, typename... Args>
  requires (std::is_same_v<T, Ts> || ...)
  void emplace(Args&&... args) {
    value.template emplace<T>(std::forward<Args...>(args)...);
  }

  template<typename T>
  bool holds_alternative() const {
    return std::holds_alternative<T>(value);
  }

  size_t index() const {
    return value.index();
  }

  Parent* operator->() {
    return std::visit([](auto& arg) -> Parent* { return &arg; }, value);
  }

  const Parent* operator->() const {
    return std::visit([](const auto& arg) -> const Parent* { return &arg; }, value);
  }

  Parent& operator*() { 
    return std::visit([](auto& arg) -> Parent& { return arg; }, value);
  }

  const Parent& operator*() const { 
    return std::visit([](const auto& arg) -> const Parent& { return arg; }, value);
  }

  template<typename T>
  requires (std::is_same_v<T, Ts> || ...)
  T& get() {
    return std::get<T>(value);
  }

  template<typename T>
  requires (std::is_same_v<T, Ts> || ...)
  const T& get() const {
    return std::get<T>(value);
  }
  
  template<typename R, typename... Args>
  R visit(Args&&... args) {
    return std::visit(overloaded{args...}, value);
  }

  template<typename R, typename... Args>
  R visit(Args&&... args) const {
    return std::visit(overloaded{args...}, value);
  }

private:

  // for simple visitor pattern
  template<class... Args>
  struct overloaded : Args... { using Args::operator()...; };

  std::variant<Ts...> value;
};

}