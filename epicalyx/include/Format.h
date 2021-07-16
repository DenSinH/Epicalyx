#pragma once

#include <stdexcept>
#include <memory>


namespace epi::calyx {

template<typename... Args>
std::string Format(const std::string& format, const Args&... args) {
  int buf_size = std::snprintf(nullptr, 0, format.c_str(), args...);
  if (buf_size <= 0) {
    throw std::runtime_error("Error during formatting of string");
  }
  auto buf = std::make_unique<char[]>((size_t)buf_size);
  std::snprintf(buf.get(), buf_size, format.c_str(), args...);
  return std::string(buf.get(), buf.get() + buf_size);
}


template<typename... Args>
std::runtime_error FormatExcept(const std::string& format, const Args&... args) {
  return std::runtime_error(Format(format, args...));
}

}