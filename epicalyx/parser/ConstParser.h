#pragma once

/*
 * Basic parser class to parse constant expressions
 * in the preprocessing stage. This is to prevent
 * having to allocate a lot of bloat (identifier maps,
 * typename maps, etc.) when parsing a preprocessor
 * constant expression
 * */

#include "Locatable.h"
#include "ast/NodeFwd.h"

namespace epi {

namespace cotyl {

template<typename T>
struct Stream;

struct CString;

}

struct AnyToken;
enum class TokenType : u32;

struct ConstParser : public cotyl::Locatable {

  ConstParser(cotyl::Stream<AnyToken>& in_stream);

  void PrintLoc(std::ostream& out) const final;
  i64 EConstexpr();

  // needs public access for shorthand parsing Binop Expressions
  virtual ast::pExpr ECast();
  template<ast::pExpr (ConstParser::*SubNode)(), enum TokenType... types>
  ast::pExpr EBinopImpl();

protected:
  virtual ast::pExpr EPrimary();
  ast::pExpr EBinop();
  ast::pExpr ETernary();
  virtual ast::pExpr EAssignment();

  cotyl::Stream<AnyToken>& in_stream;
};

}
