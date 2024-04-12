#pragma once

/*
 * Basic parser class to parse constant expressions
 * in the preprocessing stage. This is to prevent
 * having to allocate a lot of bloat (identifier maps,
 * typename maps, etc.) when parsing a preprocessor
 * constant expression
 * */

#include "Stream.h"
#include "tokenizer/Token.h"
#include "ast/Initializer.h"

namespace epi {

struct ConstParser : public cotyl::Locatable {

  ConstParser(cotyl::Stream<AnyToken>& in_stream) : in_stream{in_stream} { }

  void PrintLoc() const final;

  virtual pExpr EPrimary();
  virtual pExpr ECast();
  template<pExpr (ConstParser::*SubNode)(), enum TokenType... types>
  pExpr EBinopImpl();
  pExpr EBinop();
  pExpr ETernary();
  virtual pExpr EAssignment();
  i64 EConstexpr();

  cotyl::Stream<AnyToken>& in_stream;

  virtual pType<const CType> ResolveIdentifierType(const std::string& name) const;
};

}
