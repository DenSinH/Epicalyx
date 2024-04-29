#pragma once

#include "SStream.h"
#include "Stringify.h"
#include "Vector.h"
#include "Exceptions.h"

#include <memory>
#include <string>
#include <vector>


namespace epi::cotyl {

template<typename... Args>
std::string Format(const std::string& format, const Args& ... args) {
  int buf_size = std::snprintf(nullptr, 0, format.c_str(), args...);
  if (buf_size <= 0) {
    throw cotyl::Exception("Format Error", "Error during string formatting");
  }
  auto buf = std::make_unique<char[]>((size_t) buf_size + 1);
  std::snprintf(buf.get(), buf_size + 1, format.c_str(), args...);
  return std::string(buf.get(), buf.get() + buf_size);
}

template<typename ...Args>
std::string FormatStr(const std::string& format, const Args& ... args) {
  return Format(format, stringify(args).c_str()...);
}


template<typename T, typename... Args>
requires (std::is_base_of_v<Exception, T>)
T FormatExcept(const std::string& format, const Args& ... args) {
  return T(Format(format, args...));
}

template<typename T, typename... Args>
requires (std::is_base_of_v<Exception, T>)
T FormatExceptStr(const std::string& format, const Args& ... args) {
  return T(FormatStr(format, args...));
}

template<typename C>
std::string Join(const std::string& delimiter, const C& values) {
  if (values.empty()) return "";

  cotyl::StringStream result{};
  for (int i = 0; i < values.size() - 1; i++) {
    result << stringify(values[i]) << ", ";
  }
  result << stringify(values.back());
  return result.finalize();
}

}