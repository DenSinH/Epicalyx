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
struct Compound;
struct FunctionDefinition;

struct Parser {

  Parser(cotyl::pStream<Token>& in_stream);

  using enum_type = i32;

  pExpr EPrimary();
  pExpr EPostfix();
  pExpr EUnary();
  pType<const CType> ETypeName();
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
  pNode<Compound> SCompound();

  pNode<FunctionDefinition> ExternalDeclaration(std::vector<pNode<Decl>>& dest);

  void PushScope();
  void PopScope();

  void Parse();
  void Data();

  cotyl::pStream<Token>& in_stream;
  cotyl::Scope<std::string, enum_type> enum_values{};
  cotyl::Scope<std::string, bool> enums{};
  cotyl::Scope<std::string, pType<const CType>> typedefs{};
  cotyl::Scope<std::string, pType<const StructUnionType>> structdefs{};
  cotyl::Scope<std::string, pType<const StructUnionType>> uniondefs{};

  cotyl::Scope<std::string, pType<const CType>> variables{};

  // external results
  std::vector<pNode<FunctionDefinition>> functions{};
  std::vector<pNode<Decl>> declarations{};
};

}
