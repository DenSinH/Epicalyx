#pragma once

#include "Stream.h"
#include "Scope.h"
#include "tokenizer/Token.h"
#include "types/Types.h"
#include "taxy/Initializer.h"

#include <stack>

namespace epi {

namespace taxy {

struct Declaration;
enum class StorageClass;
struct Compound;
struct FunctionDefinition;

}

using namespace taxy;


struct Parser final : public cotyl::Locatable {

  Parser(cotyl::Stream<pToken>& in_stream);

  void PrintLoc() const final;

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
  void DInitDeclaratorList(std::vector<pNode<Declaration>>& dest);
  bool IsDeclarationSpecifier(int after = 0);

  pNode<Stat> SStatement();
  pNode<Compound> SCompound();

  pNode<FunctionDefinition> ExternalDeclaration(std::vector<pNode<Declaration>>& dest);

  void PushScope();
  void PopScope();

  void Parse();
  void Data();

  enum class Loop {
    For, While, DoWhile
  };

  cotyl::Stream<pToken>& in_stream;
  cotyl::MapScope<std::string, enum_type> enum_values{};
  cotyl::SetScope<std::string> enums{};
  cotyl::MapScope<std::string, pType<const CType>> typedefs{};
  cotyl::MapScope<std::string, pType<const StructUnionType>> structdefs{};
  cotyl::MapScope<std::string, pType<const StructUnionType>> uniondefs{};

  pType<const CType> function_return{};

  std::deque<Loop> loop_scope{};
  cotyl::SetScope<i64> case_scope{};
  std::unordered_set<std::string> labels{};
  std::unordered_set<std::string> unresolved_labels{};
  cotyl::MapScope<std::string, pType<const CType>> variables{};

  // external results
  std::vector<pNode<Decl>> declarations{};
};

}
