#pragma once

#include <variant>
#include <type_traits>

#include "Is.h"


namespace epi::cotyl {

template<typename Parent, typename... Ts>
requires (std::is_base_of_v<Parent, Ts> && ...)
struct Variant {

  template<typename T>
  Variant(const T& val) : value{val} { }
  template<typename T>
  Variant(T&& val) : value{std::move(val)} { }

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
    
private:
  std::variant<Ts...> value;
};

}