#pragma once

#include "Variant.h"

#include <string>
#include <memory>


#define STRINGIFY_METHOD(...) std::string stringify(const __VA_ARGS__& value)

namespace epi {

template<typename T> 
static STRINGIFY_METHOD(std::unique_ptr<T>) { 
  return stringify(*value); 
}

template<typename T>
static STRINGIFY_METHOD(std::shared_ptr<T>) {
  return stringify(*value); 
}

template<typename T, typename... Ts> 
static STRINGIFY_METHOD(cotyl::Variant<T, Ts...>) { 
  return value.template visit<std::string>(
    [](const auto& value) -> std::string { return stringify(value); }
  ); 
}

static STRINGIFY_METHOD(std::string) { 
  return value; 
}

template<typename T>
static STRINGIFY_METHOD(T) {
  if constexpr(std::is_arithmetic_v<T>) {
    return std::to_string(value);
  }
  else {
    return value.ToString();
  }
}

}