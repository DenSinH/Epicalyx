#pragma once

#include "Default.h"
#include "Stringify.h"

#include <memory>
#include <cstring>
#include <string>

namespace epi::cotyl {

struct CString {

  using size_type = std::size_t;
  
  CString() : size_{0}, direct{} { }

  explicit CString(char c) : 
      size_{1}, direct{c} { }

  explicit CString(size_type size, const char* str) : size_{size} {
    if (!is_direct()) {
      indirect = std::make_unique<char[]>(size_ + 1);
    }
    std::memcpy(c_str(), str, size_);
  }

  ~CString() {
    if (!is_direct()) {
      indirect.~unique_ptr();
    }
  }

  explicit CString(const char* str) : 
      CString{(size_type)strlen(str), str} { }
  
  explicit CString(std::string&& str) : 
      CString{(size_type)str.size(), str.c_str()} { }

  explicit CString(const std::string& str) : 
      CString{(size_type)str.size(), str.c_str()} { }

  explicit CString(const std::string_view& str) : 
      CString{(size_type)str.size(), str.data()} { }

  explicit CString(const CString& other) :
      CString{other.size_, other.c_str()} { }

  CString(CString&& other) : size_{other.size_} {
    if (is_direct()) {
      // copying all data may compile to a single mov
      std::memcpy(c_str(), other.c_str(), direct_data_size);
    }
    else {
      indirect = std::move(other.indirect);
    }
  }

  CString& operator=(const CString& other) = delete;
  CString& operator=(CString&& other) {
    // destruct self 
    this->~CString();
    new (this) CString{std::move(other)};
    return *this;
  }

  const char* c_str() const {
    if (is_direct()) return direct;
    return indirect.get(); 
  }
  
  char* c_str() { 
    if (is_direct()) return direct;
    return indirect.get(); 
  }

  using const_iterator = const char*;
  using iterator = char*;

  const_iterator begin() const { return c_str(); }
  const_iterator end() const { return c_str() + size_; }
  iterator begin() { return c_str(); }
  iterator end() { return c_str() + size_; }
  std::size_t size() const { return size_; }
  bool empty() const { return size_ == 0; }

  std::string_view view() const {
    return {c_str(), size_};
  }

  operator std::string_view() const {
    return view();
  }

  std::string str() const {
    return std::string(view());
  }

  char& operator[](size_type pos) {
    return c_str()[pos];
  }
  
  const char& operator[](size_type pos) const {
    return c_str()[pos];
  }

  bool operator==(const CString& other) const {
    if (size_ != other.size_) return false;
    return std::memcmp(c_str(), other.c_str(), size_) == 0;
  }

  bool streq(const std::string& other) const {
    if (size_ != other.size()) return false;
    return std::memcmp(c_str(), other.c_str(), size_) == 0;
  }

private:
  static constexpr size_type direct_data_size = (size_type)(sizeof(std::unique_ptr<char[]>));
  
  bool is_direct() const {
    return size_ < direct_data_size;
  }
  
  union {
    std::unique_ptr<char[]> indirect;
    char direct[direct_data_size]{};
  };
  size_type size_;
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