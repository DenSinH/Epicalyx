#pragma once

#include "Format.h"

#include "taxy/Node.h"
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
  Invalid,
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

extern const cotyl::unordered_map<const std::string, TokenType> Punctuators;
extern const cotyl::unordered_map<const std::string, TokenType> Keywords;
extern const cotyl::unordered_map<TokenType, TokenInfo> TokenData;


struct Token {
  TokenType type;

  Token() : type(TokenType::Invalid) { }

  Token(TokenType type) : type(type) {

  }

  virtual taxy::pExpr GetConst(ConstTokenVisitor& v) const { return v.Visit(*this); }
  void Expect(TokenType t) const;

  virtual std::string ToString() const {
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

  taxy::pExpr GetConst(ConstTokenVisitor& v) const final { return v.Visit(*this); }

  std::string ToString() const final {
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

  taxy::pExpr GetConst(ConstTokenVisitor& v) const final { return v.Visit(*this); }

  std::string ToString() const final {
    return std::to_string(value);
  }

  TokenClass Class() final {
    return TokenClass::NumericalConstant;
  }

  const T value;
};


struct tStringConstant : public Token {
  explicit tStringConstant(const std::string value) :
      Token(TokenType::StringConstant),
      value(value) {

  }

  taxy::pExpr GetConst(ConstTokenVisitor& v) const final { return v.Visit(*this); }

  std::string ToString() const final {
    return cotyl::Format("\"%s\"", value.c_str());
  }

  TokenClass Class() final {
    return TokenClass::StringConstant;
  }

  const std::string value;
};

using pToken = std::unique_ptr<Token>;

}