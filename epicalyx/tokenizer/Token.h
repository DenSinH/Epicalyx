#pragma once

#include "Default.h"
#include "Stringify.h"
#include "Escape.h"
#include "Format.h"
#include "Variant.h"

#include "Containers.h"

#include <string>
#include <utility>


namespace epi {

enum class TokenClass {
  Identifier,
  Keyword,
  Punctuator,
  StringConstant,
  NumericalConstant,
};

enum class TokenType {
  Identifier,

  // constants
  NumericConstant, StringConstant,

  // Keywords
  Auto, Break, Case, Char, Const, Continue, Default, Do,
  Double, Else, Enum, Extern, Float, For, Goto, If, Inline,
  Int, Long, Register, Restrict, Return, Short, Signed, Sizeof,
  Static, Struct, Switch, Typedef, Union, Unsigned, Void, Volatile,
  While, Alignas, Alignof, Atomic, Bool, Complex, Generic, Imaginary,
  Noreturn, StaticAssert, ThreadLocal,

  // Punctuators
  LBracket,       // [
  RBracket,       // ]
  LParen,         // (
  RParen,         // )
  LBrace,         // {
  RBrace,         // }
  Dot,            // .
  Arrow,          // ->
  Incr,           // ++
  Decr,           // --
  Ampersand,      // &
  Asterisk,       // *
  Plus,           // +
  Minus,          // -
  Tilde,          // ~
  Exclamation,    // !
  Div,            // /
  Mod,            // %
  LShift,         // <<
  RShift,         // >>
  Less,           // <
  Greater,        // >
  LessEqual,      // <=
  GreaterEqual,   // >=
  Equal,          // ==
  NotEqual,       // !=
  BinXor,         // ^
  BinOr,          // |
  LogicalAnd,     // &&
  LogicalOr,      // ||
  Question,       // ?
  Colon,          // :
  SemiColon,      // ;
  Ellipsis,       // ...
  Assign,         // =
  IMul,           // *=
  IDiv,           // /=
  IMod,           // %=
  IPlus,          // +=
  IMinus,         // -=
  ILShift,        // <<=
  IRShift,        // >>=
  IAnd,           // &=
  IXor,           // ^=
  IOr,            // |=
  Comma,          // ,
  Hashtag,        // #
  HHashtag,       // ##
  // <: :> <% %> %: %:%: I don't know what these are
};

struct TokenInfo {
  std::string name;
  TokenClass cls;
};

STRINGIFY_METHOD(TokenType);

struct Token {
  TokenClass cls;
  TokenType type;

  Token(TokenClass cls, TokenType type) : cls{cls}, type{type} {

  }

  bool operator==(const Token& other) const {
    return type == other.type;
  }

  bool operator==(const TokenType& type) const {
    return this->type == type;
  }
};

struct PunctuatorToken final : public Token {
  PunctuatorToken(TokenType type) : Token{TokenClass::Punctuator, type} { }
};

static STRINGIFY_METHOD(PunctuatorToken) { return stringify(value.type); }

struct KeywordToken final : public Token {
  KeywordToken(TokenType type) : Token{TokenClass::Keyword, type} { }
};

static STRINGIFY_METHOD(KeywordToken) { return stringify(value.type); }

struct IdentifierToken final : public Token {
  explicit IdentifierToken(std::string  name) :
    Token(TokenClass::Identifier, TokenType::Identifier),
    name(std::move(name)) {

  }

  const std::string name;
};

static STRINGIFY_METHOD(IdentifierToken) { return value.name; }


template<typename T>
struct NumericalConstantToken final : public Token {
  explicit NumericalConstantToken(T value) :
      Token(TokenClass::NumericalConstant, TokenType::NumericConstant),
      value(value) {

  }

  const T value;
};

template<typename T>
static STRINGIFY_METHOD(NumericalConstantToken<T>) { return stringify(value.value); }

struct StringConstantToken : public Token {
  explicit StringConstantToken(const std::string value) :
      Token(TokenClass::StringConstant, TokenType::StringConstant),
      value(value) {

  }

  const std::string value;
};

static STRINGIFY_METHOD(StringConstantToken) {
  return cotyl::Format("\"%s\"", cotyl::Escape(value.value).c_str());
}

using AnyToken = cotyl::Variant<Token,
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