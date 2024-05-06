#pragma once

#include "Default.h"
#include "Stringify.h"

#include <memory>
#include <cstring>
#include <string>

namespace epi::cotyl {

struct CString {

  using size_type = std::size_t;

  CString() : size_{0}, data{nullptr} { }

  explicit CString(char c) : 
      size_{1},
      data(std::make_unique<char[]>(size_ + 1)) {
    data.get()[0] = c;
  }

  explicit CString(const char* str) : 
      size_{(size_type)strlen(str)},
      data(std::make_unique<char[]>(size_ + 1)) {
    std::memcpy(data.get(), str, size_);
  }
  
  explicit CString(std::string&& str) : 
      size_{(size_type)str.size()},
      data(std::make_unique<char[]>(size_ + 1)) {
    std::memcpy(data.get(), str.c_str(), size_);
  }

  explicit CString(const std::string& str) : 
      size_{(size_type)str.size()},
      data(std::make_unique<char[]>(size_ + 1)) {
    std::memcpy(data.get(), str.c_str(), size_);
  }

  explicit CString(const std::string_view& str) : 
      size_{(size_type)str.size()},
      data(std::make_unique<char[]>(size_ + 1)) {
    std::memcpy(data.get(), str.data(), size_);
  }

  explicit CString(const CString& other) :
      size_{other.size_},
      data(std::make_unique<char[]>(size_ + 1)) {
    std::memcpy(data.get(), other.data.get(), size_);

  }

  CString(CString&& other) : 
      size_{other.size_}, 
      data(std::move(other.data)) {

  }

  CString& operator=(const CString& other) = delete;

  CString& operator=(CString&& other) noexcept {
    size_ = other.size_;
    data = std::move(other.data);
    return *this;
  }

  ~CString() = default;

  const char* c_str() const { return data.get(); }
  char* c_str() { return data.get(); }

  using const_iterator = const char*;
  using iterator = char*;

  const_iterator begin() const { return data.get(); }
  const_iterator end() const { return data.get() + size_; }
  iterator begin() { return data.get(); }
  iterator end() { return data.get() + size_; }
  std::size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }

  std::string_view view() const {
    return {data.get(), size_};
  }

  operator std::string_view() const {
    return view();
  }

  std::string str() const {
    return std::string(view());
  }

  char& operator[](size_type pos) {
    return data.get()[pos];
  }
  
  const char& operator[](size_type pos) const {
    return data.get()[pos];
  }

  bool operator==(const CString& other) const {
    if (size_ != other.size_) return false;
    return std::memcmp(data.get(), other.data.get(), size_) == 0;
  }

  bool streq(const char* other) const {
    if (size_ != strlen(other)) return false;
    return std::memcmp(data.get(), other, size_) == 0;
  }

private:

  size_type size_;
  std::unique_ptr<char[]> data;
};

}

namespace epi {

static STRINGIFY_METHOD(cotyl::CString) {
  return value.str();
}

}

namespace std {

template<>
struct hash<epi::cotyl::CString> {
  size_t operator()(const epi::cotyl::CString& str) const {
    return std::hash<std::string_view>{}(str.view());
  }
};

}