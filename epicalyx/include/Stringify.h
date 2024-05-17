#pragma once

#include "Variant.h"

#include <string>
#include <memory>


#define STRINGIFY_METHOD(...) std::string stringify(const __VA_ARGS__& value)

namespace epi {

template<typename T> 
inline STRINGIFY_METHOD(std::unique_ptr<T>) { 
  return stringify(*value); 
}

template<typename T>
inline STRINGIFY_METHOD(std::shared_ptr<T>) {
  return stringify(*value); 
}

template<typename T, typename... Ts> 
inline STRINGIFY_METHOD(cotyl::Variant<T, Ts...>) { 
  return value.template visit<std::string>(
    [](const auto& value) -> std::string { return stringify(value); }
  ); 
}

inline STRINGIFY_METHOD(std::string) { 
  return value; 
}

namespace detail {
  
template<typename T>
concept HasStdToString = requires(T a) {
  { std::to_string(a) } -> std::same_as<std::string>;
};

template<typename T>
concept HasToString = requires(T a) {
  { a.ToString() } -> std::same_as<std::string>;
};

}

template<detail::HasStdToString T>
inline STRINGIFY_METHOD(T) {
  return std::to_string(value);
}

template<detail::HasToString T>
inline STRINGIFY_METHOD(T) {
  return value.ToString();
}

}