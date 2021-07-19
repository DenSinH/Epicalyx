#pragma once

#include <stdexcept>
#include <memory>

#include "Is.h"


namespace epi::cotyl {

template<typename... Args>
std::string Format(const std::string& format, const Args&... args) {
  int buf_size = std::snprintf(nullptr, 0, format.c_str(), args...);
  if (buf_size <= 0) {
    throw std::runtime_error("Error during formatting of string");
  }
  auto buf = std::make_unique<char[]>((size_t)buf_size + 1);
  std::snprintf(buf.get(), buf_size + 1, format.c_str(), args...);
  return std::string(buf.get(), buf.get() + buf_size + 1);
}


template<typename ...Args>
std::string FormatStr(const std::string& format, const Args&... args) {
  static_assert(are_all_same_v<std::string, Args...>);
  return Format(format, args.c_str()...);
}


template<typename... Args>
std::runtime_error FormatExcept(const std::string& format, const Args&... args) {
  return std::runtime_error(Format(format, args...));
}

template<typename... Args>
std::runtime_error FormatExceptStr(const std::string& format, const Args&... args) {
  return std::runtime_error(FormatStr(format, args...));
}

}