#pragma once

#include "Stream.h"
#include "Scope.h"
#include "tokenizer/Token.h"
#include "types/Types.h"
#include "nodes/Initializer.h"

#include <stack>

namespace epi {

struct Declaration;
enum class StorageClass;
struct InitDeclaration;

struct Parser {

  Parser(cotyl::pStream<Token>& in_stream) :
      in_stream(in_stream) {

  }

  using enum_type = i64;

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

  Initializer EInitializer();
  pNode<InitializerList> EInitializerList();

  void DStaticAssert();
  std::pair<pType<>, StorageClass> DSpecifier();
  pType<> DEnum();
  pType<> DStruct();
  std::string DDirectDeclaratorImpl(std::stack<pType<PointerType>>& dest);
  pNode<Declaration> DDeclarator(pType<> ctype, StorageClass storage);
  void DInitDeclaratorList(std::vector<pNode<InitDeclaration>>& dest);
  bool IsDeclarationSpecifier(int after = 0);

  pNode<Stat> SStatement();
  pNode<Stat> SCompound();

  cotyl::pStream<Token>& in_stream;
  cotyl::Scope<std::string, enum_type> enum_values{};
  cotyl::Scope<std::string, bool> enums{};
  cotyl::Scope<std::string, pType<const CType>> typedefs{};
  cotyl::Scope<std::string, pType<const CType>> structdefs{};
  cotyl::Scope<std::string, pType<const CType>> uniondefs{};
};

}
