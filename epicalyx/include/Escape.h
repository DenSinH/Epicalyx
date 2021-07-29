#pragma once

#include <string>
#include <sstream>


namespace epi::cotyl {

static std::string Escape(const std::string& in) {
  std::stringstream result{};
  for (const auto c : in) {
    switch (c) {
      case '\n': result << "\\n"; break;
      case '\r': result << "\\r"; break;
      case '\t': result << "\\t"; break;
      case '\'': result << "\\'"; break;
      case '\"': result << "\\\""; break;
      case '\\': result << "\\\\"; break;
      case '\a': result << "\\a"; break;
      case '\b': result << "\\b"; break;
      case '\f': result << "\\f"; break;
      case '\?': result << "\\?"; break;
      case '\v': result << "\\v"; break;
      default: result << c; break;
    }
  }
  return result.str();
}

}