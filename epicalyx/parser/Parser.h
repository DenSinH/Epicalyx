#pragma once

#include "Stream.h"
#include "tokenizer/Token.h"
#include "types/Types.h"

#include <stack>

namespace epi {

struct Declaration;
struct InitDeclaration;

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
  pExpr EExpression();
  void EExpressionList(std::vector<pExpr>& dest);

  void DStaticAssert();
  pType<> DSpecifier();
  std::string DDirectDeclaratorImpl(std::stack<pType<PointerType>>& dest);
  pNode<Declaration> DDeclarator(pType<> ctype);
  void DInitDeclaratorList(std::vector<pNode<Declaration>>& dest);
  bool IsDeclarationSpecifier();

  pNode<Stat> SStatement();
  pNode<Stat> SCompound();

  cotyl::pStream<Token>& in_stream;
};

}
