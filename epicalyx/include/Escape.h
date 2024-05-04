#pragma once

#include "SStream.h"

#include <string>


namespace epi::cotyl {

static cotyl::CString Escape(const char* in) {
  cotyl::StringStream result{};
  for (auto c = in; *c; c++) {
    switch (*c) {
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
      default: result << *c; break;
    }
  }
  return result.cfinalize();
}

}