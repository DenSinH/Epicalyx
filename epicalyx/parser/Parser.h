#pragma once

#include "Stream.h"
#include "tokenizer/Token.h"
#include "types/Types.h"

namespace epi {

struct Parser {

  Parser(calyx::pStream<Token>& in_stream) :
      in_stream(in_stream) {

  }

  pExpr EPrimary();
  pExpr EPostfix();
  pExpr EUnary();
  pExpr ECast();
  template<pType<> (CType::*op)(const CType& other), pExpr (Parser::*side)()>
  pExpr EBinop();
  pExpr EAssignment();
  pExpr EExpression();

  calyx::pStream<Token>& in_stream;
};

}
