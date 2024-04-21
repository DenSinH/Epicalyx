#pragma once

#include "SStream.h"
#include "Stringify.h"

#include <stdexcept>
#include <memory>
#include <string>
#include "Vector.h"


namespace epi::cotyl {

template<typename... Args>
std::string Format(const std::string& format, const Args& ... args) {
  int buf_size = std::snprintf(nullptr, 0, format.c_str(), args...);
  if (buf_size <= 0) {
    throw std::runtime_error("Error during formatting of string");
  }
  auto buf = std::make_unique<char[]>((size_t) buf_size + 1);
  std::snprintf(buf.get(), buf_size + 1, format.c_str(), args...);
  return std::string(buf.get(), buf.get() + buf_size);
}

template<typename ...Args>
std::string FormatStr(const std::string& format, const Args& ... args) {
  return Format(format, stringify(args).c_str()...);
}


template<typename... Args>
std::runtime_error FormatExcept(const std::string& format, const Args& ... args) {
  return std::runtime_error(Format(format, args...));
}

template<typename... Args>
std::runtime_error FormatExceptStr(const std::string& format, const Args& ... args) {
  return std::runtime_error(FormatStr(format, args...));
}

template<typename T>
std::string Join(const std::string& delimiter, const cotyl::vector<T>& values) {
  if (values.empty()) return "";

  cotyl::StringStream result{};
  for (int i = 0; i < values.size() - 1; i++) {
    result << stringify(values[i]) << ", ";
  }
  result << stringify(values.back());
  return result.finalize();
}

}