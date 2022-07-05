#pragma once

#include "Stream.h"


namespace epi::detail {

constexpr bool is_valid_ident_start(char c) {
  return std::isalpha(c) || c == '_';
}

constexpr bool is_valid_ident_char(char c) {
  return std::isalnum(c) || c == '_';
}

static std::string get_identifier(cotyl::Stream<char>& stream) {
  std::stringstream identifier{};
  if (stream.PredicateAfter(0, detail::is_valid_ident_start)) {
    while (stream.PredicateAfter(0, detail::is_valid_ident_char)) {
      identifier << stream.Get();
    }
  }
  else {
    throw std::runtime_error("Expected identifier");
  }
  return identifier.str();
}

}