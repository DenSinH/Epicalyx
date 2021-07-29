#pragma once

#include <stdexcept>
#include <memory>
#include <string>
#include <sstream>
#include <vector>


namespace epi::cotyl {

template<typename T>
static std::string to_string(const T& t) {
  if constexpr(std::is_arithmetic_v<T>) {
    return std::to_string(t);
  }
  else {
    return t.ToString();
  }
}

template<typename T> static std::string to_string(const std::unique_ptr<T>& t) { return t->ToString(); }
template<typename T> static std::string to_string(const std::shared_ptr<T>& t) { return t->ToString(); }
static std::string to_string(const std::string& s) { return s; }

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
  using epi::cotyl::to_string;
  return Format(format, to_string(args).c_str()...);
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
std::string Join(const std::string& delimiter, const std::vector<T>& values) {
  if (values.empty()) return "";

  std::stringstream result{};
  for (int i = 0; i < values.size() - 1; i++) {
    result << values[i].ToString() << ", ";
  }
  result << values.back().ToString();
  return result.str();
}

template<typename T>
std::string Join(const std::string& delimiter, const std::vector<std::unique_ptr<T>>& values) {
  if (values.empty()) return "";

  std::stringstream result{};
  for (int i = 0; i < values.size() - 1; i++) {
    result << values[i]->ToString() << ", ";
  }
  result << values.back()->ToString();
  return result.str();
}

template<typename T>
std::string Join(const std::string& delimiter, const std::vector<std::shared_ptr<T>>& values) {
  if (values.empty()) return "";

  std::stringstream result{};
  for (int i = 0; i < values.size() - 1; i++) {
    result << values[i]->to_string() << ", ";
  }
  result << values.back()->to_string();
  return result.str();
}

}