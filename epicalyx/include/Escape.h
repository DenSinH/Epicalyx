#pragma once

#include "Default.h"
#include "SStream.h"
#include "Stream.h"
#include "CString.h"

#include <array>


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

static constexpr std::array<i32, 0x100> ASCIIHexToInt = {
        // ASCII
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
         0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
};

static void Unescape(cotyl::StringStream& dest, cotyl::Stream<char>& src) {
  char c = src.Get();
  switch (c) {
    case '\'': case '\"': case '\\': dest << c; break;
    case 'a': dest << '\a'; break;
    case 'b': dest << '\b'; break;
    case 'f': dest << '\f'; break;
    case 'n': dest << '\n'; break;
    case 'r': dest << '\r'; break;
    case 't': dest << '\t'; break;
    case 'v': dest << '\v'; break;
    case '?': dest << '\?'; break;
    case 'x': {
      // hex literal
      char hex = 0;
      while (src.PredicateAfter(0, std::isxdigit)) {
        c = src.Get();
        hex <<= 4;
        hex |= ASCIIHexToInt[c];
      }
      dest << hex;
      break;
    }
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7': {
      // octal literal
      char oct = c - '0';
      for (int count = 0; src.PredicateAfter(0, std::isdigit) && count < 3; count++) {
        if (src.Peek(c) && c < '8') {
          c = src.Get();
          oct <<= 3;
          oct |= c - '0';
        }
      }
      dest << oct;
      break;
    }
    default:
      // invalid escape
      dest << '\\' << c;
      break;
  }
}

}