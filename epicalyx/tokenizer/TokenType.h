#pragma once 

#include "Stringify.h"


namespace epi {

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

STRINGIFY_METHOD(TokenType);

}