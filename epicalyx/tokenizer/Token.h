#pragma once

#include "Default.h"
#include "Stringify.h"
#include "Variant.h"
#include "TokenType.h"

#include <string>
#include <utility>


namespace epi {

struct Token {
  TokenType type;

  Token(TokenType type) : type{type} {

  }

  bool operator==(const Token& other) const {
    return type == other.type;
  }

  bool operator==(const TokenType& type) const {
    return this->type == type;
  }
};

struct PunctuatorToken final : public Token {
  PunctuatorToken(TokenType type) : Token{type} { }
};

STRINGIFY_METHOD(PunctuatorToken);

struct KeywordToken final : public Token {
  KeywordToken(TokenType type) : Token{type} { }
};

STRINGIFY_METHOD(KeywordToken);

struct IdentifierToken final : public Token {
  explicit IdentifierToken(std::string  name) :
    Token(TokenType::Identifier),
    name(std::move(name)) {

  }

  const std::string name;
};

STRINGIFY_METHOD(IdentifierToken);

template<typename T>
struct NumericalConstantToken final : public Token {
  explicit NumericalConstantToken(T value) :
      Token(TokenType::NumericConstant),
      value(value) {

  }

  const T value;
};

template<typename T>
STRINGIFY_METHOD(NumericalConstantToken<T>);

struct StringConstantToken : public Token {
  explicit StringConstantToken(const std::string value) :
      Token(TokenType::StringConstant),
      value(value) {

  }

  const std::string value;
};

STRINGIFY_METHOD(StringConstantToken);

namespace detail {

using any_token_t = cotyl::Variant<Token,
  PunctuatorToken,
  KeywordToken,
  IdentifierToken,
  NumericalConstantToken<i32>,
  NumericalConstantToken<u32>,
  NumericalConstantToken<i64>,
  NumericalConstantToken<u64>,
  NumericalConstantToken<float>,
  NumericalConstantToken<double>,
  StringConstantToken
>;

}

struct AnyToken : detail::any_token_t {
  using base_t = detail::any_token_t;
  using detail::any_token_t::any_token_t;
};

STRINGIFY_METHOD(AnyToken);

}