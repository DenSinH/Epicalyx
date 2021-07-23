#pragma once

#include "Stream.h"
#include "Scope.h"
#include "tokenizer/Token.h"
#include "types/Types.h"

#include <stack>

namespace epi {

struct Declaration;
enum class StorageClass;
struct InitDeclaration;
struct InitializerList;

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
  i64 EConstexpr();
  void EExpressionList(std::vector<pExpr>& dest);

  pNode<InitializerList> EInitializerList();

  void DStaticAssert();
  std::pair<pType<>, StorageClass> DSpecifier();
  std::string DDirectDeclaratorImpl(std::stack<pType<PointerType>>& dest);
  pNode<Declaration> DDeclarator(pType<> ctype, StorageClass storage);
  void DInitDeclaratorList(std::vector<pNode<InitDeclaration>>& dest);
  bool IsDeclarationSpecifier(int after = 0);

  pNode<Stat> SStatement();
  pNode<Stat> SCompound();

  cotyl::pStream<Token>& in_stream;
  cotyl::Scope<std::string, i64> constants{};
  cotyl::Scope<std::string, pType<const CType>> typedefs{};
};

}
