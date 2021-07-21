#pragma once

#include "Stream.h"
#include "tokenizer/Token.h"
#include "types/Types.h"

#include <stack>

namespace epi {

struct Declarator;

struct Parser {

  Parser(cotyl::pStream<Token>& in_stream) :
      in_stream(in_stream) {

  }

  pExpr EPrimary();
  pExpr EPostfix();
  pExpr EUnary();
  pExpr ECast();
  template<pExpr (Parser::*SubNode)(), enum TokenType... types>
  pExpr EBinopImpl();
  pExpr EBinop();
  pExpr ETernary();
  pExpr EAssignment();

  void DStaticAssert();
  pType<> DSpecifier();
  std::string DDirectDeclaratorImpl(std::stack<pType<PointerType>>& dest);
  pNode<Declarator> DDeclarator();

  cotyl::pStream<Token>& in_stream;
};

}
