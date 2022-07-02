#pragma once

#include "ConstParser.h"
#include "Scope.h"
#include "Containers.h"
#include "types/Types.h"

#include <stack>

namespace epi {

namespace ast {

struct Declaration;
enum class StorageClass;
struct Compound;
struct FunctionDefinition;

}

using namespace ast;


struct Parser final : public ConstParser {
  using ConstParser::ConstParser;

  using enum_type = i32;

  pExpr EPrimary() final;
  pExpr EPostfix();
  pExpr EUnary();
  pType<const CType> ETypeName();
  pExpr ECast() final;
  pExpr EAssignment() final;
  pExpr EExpression();
  Initializer EInitializer();
  pNode<InitializerList> EInitializerList();
  pType<const CType> ResolveIdentifierType(const std::string& name) const final;
  void EExpressionList(std::vector<pExpr>& dest);

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

  cotyl::MapScope<std::string, enum_type> enum_values{};
  cotyl::SetScope<std::string> enums{};
  cotyl::MapScope<std::string, pType<const CType>> typedefs{};
  cotyl::MapScope<std::string, pType<const StructUnionType>> structdefs{};
  cotyl::MapScope<std::string, pType<const StructUnionType>> uniondefs{};

  pType<const CType> function_return{};

  std::deque<Loop> loop_scope{};
  cotyl::SetScope<i64> case_scope{};
  cotyl::unordered_set<std::string> labels{};
  cotyl::unordered_set<std::string> unresolved_labels{};
  cotyl::MapScope<std::string, pType<const CType>> variables{};

  // external results
  std::vector<pNode<Decl>> declarations{};
};

}
