#pragma once

#include "Format.h"

#include "parser/nodes/Node.h"
#include "parser/ConstTokenVisitor.h"

#include <string>
#include <utility>
#include <map>


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
  FloatConstant, NumericConstant, StringConstant,

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

extern const std::map<const std::string, TokenType> Punctuators;
extern const std::map<const std::string, TokenType> Keywords;
extern const std::map<TokenType, TokenInfo> TokenData;


struct Token {
  const TokenType type;

  explicit Token(TokenType type) : type(type) {

  }

  virtual pExpr GetConst(ConstTokenVisitor& v) const { return v.Visit(*this); }
  void Expect(TokenType t) const;

  virtual std::string to_string()  const {
    return TokenData.at(type).name;
  }

  bool operator==(const Token& other) const {
    return type == other.type;
  }

  virtual TokenClass Class() {
    return TokenData.at(type).cls;
  }
};

struct tIdentifier final : public Token {
  explicit tIdentifier(std::string  name) :
    Token(TokenType::Identifier),
    name(std::move(name)) {

  }

  pExpr GetConst(ConstTokenVisitor& v) const final { return v.Visit(*this); }

  std::string to_string()  const final {
    return name;
  }

  TokenClass Class() final {
    return TokenClass::Identifier;
  }

  const std::string name;
};


template<typename T>
struct tNumericConstant final : public Token {
  explicit tNumericConstant(T value) :
      Token(TokenType::NumericConstant),
      value(value) {

  }

  pExpr GetConst(ConstTokenVisitor& v) const final { return v.Visit(*this); }

  std::string to_string()  const final {
    return std::to_string(value);
  }

  TokenClass Class() final {
    return TokenClass::NumericalConstant;
  }

  const T value;
};


struct tStringConstant : public Token {
  explicit tStringConstant(const std::string value) :
      Token(TokenType::FloatConstant),
      value(value) {

  }

  pExpr GetConst(ConstTokenVisitor& v) const final { return v.Visit(*this); }

  std::string to_string()  const final {
    return calyx::Format("\"%s\"", value.c_str());
  }

  TokenClass Class() final {
    return TokenClass::StringConstant;
  }

  const std::string value;
};

static std::string to_string(const Token& t) {
  return t.to_string();
}

using pToken = std::shared_ptr<Token>;

}