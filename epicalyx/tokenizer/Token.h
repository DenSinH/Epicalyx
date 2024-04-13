#pragma once

#include "Stringify.h"
#include "Escape.h"
#include "Format.h"
#include "Variant.h"

#include "ast/Node.h"
#include "parser/ConstTokenVisitor.h"
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

  virtual ast::pExpr GetConst(ConstTokenVisitor& v) const { return v.Visit(*this); }

  virtual std::string ToString() const {
    return stringify(type);
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

struct KeywordToken final : public Token {
  KeywordToken(TokenType type) : Token{TokenClass::Keyword, type} { }
};

struct IdentifierToken final : public Token {
  explicit IdentifierToken(std::string  name) :
    Token(TokenClass::Identifier, TokenType::Identifier),
    name(std::move(name)) {

  }

  ast::pExpr GetConst(ConstTokenVisitor& v) const final { return v.Visit(*this); }

  std::string ToString() const final {
    return name;
  }

  const std::string name;
};


template<typename T>
struct NumericalConstantToken final : public Token {
  explicit NumericalConstantToken(T value) :
      Token(TokenClass::NumericalConstant, TokenType::NumericConstant),
      value(value) {

  }

  ast::pExpr GetConst(ConstTokenVisitor& v) const final { return v.Visit(*this); }

  std::string ToString() const final {
    return stringify(value);
  }

  const T value;
};


struct StringConstantToken : public Token {
  explicit StringConstantToken(const std::string value) :
      Token(TokenClass::StringConstant, TokenType::StringConstant),
      value(value) {

  }

  ast::pExpr GetConst(ConstTokenVisitor& v) const final { return v.Visit(*this); }

  std::string ToString() const final {
    return cotyl::Format("\"%s\"", cotyl::Escape(value).c_str());
  }

  const std::string value;
};

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

template<typename T, typename... Args>
AnyToken MakeAnyToken(const Args&... args) {
  return AnyToken(T{args...});
}

}