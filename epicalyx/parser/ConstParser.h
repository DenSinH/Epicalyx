#pragma once

/*
 * Basic parser class to parse constant expressions
 * in the preprocessing stage. This is to prevent
 * having to allocate a lot of bloat (identifier maps,
 * typename maps, etc.) when parsing a preprocessor
 * constant expression
 * */

#include "Locatable.h"
#include "ast/Initializer.h"

namespace epi {

namespace cotyl {

template<typename T>
struct Stream;

struct CString;

}

struct AnyToken;
enum class TokenType : u32;

struct ConstParser : public cotyl::Locatable {

  ConstParser(cotyl::Stream<AnyToken>& in_stream) : in_stream{in_stream} { }

  void PrintLoc() const final;

  virtual ast::pExpr EPrimary();
  virtual ast::pExpr ECast();
  template<ast::pExpr (ConstParser::*SubNode)(), enum TokenType... types>
  ast::pExpr EBinopImpl();
  ast::pExpr EBinop();
  ast::pExpr ETernary();
  virtual ast::pExpr EAssignment();
  i64 EConstexpr();

  cotyl::Stream<AnyToken>& in_stream;

  virtual pType<const CType> ResolveIdentifierType(const cotyl::CString& name) const;
};

}
