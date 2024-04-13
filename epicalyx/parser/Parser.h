#pragma once

#include "ConstParser.h"
#include "Scope.h"
#include "Containers.h"
#include "types/Types.h"

#include <stack>

namespace epi {

namespace ast {

struct DeclarationNode;
enum class StorageClass;
struct CompoundNode;
struct FunctionDefinitionNode;

}


struct Parser final : public ConstParser {
  using ConstParser::ConstParser;

  using enum_type = i32;

  ast::pExpr EPrimary() final;
  ast::pExpr EPostfix();
  ast::pExpr EUnary();
  pType<const CType> ETypeName();
  ast::pExpr ECast() final;
  ast::pExpr EAssignment() final;
  ast::pExpr EExpression();
  ast::Initializer EInitializer();
  ast::pNode<ast::InitializerList> EInitializerList();
  pType<const CType> ResolveIdentifierType(const std::string& name) const final;
  void EExpressionList(std::vector<ast::pExpr>& dest);

  void DStaticAssert();
  std::pair<pType<>, ast::StorageClass> DSpecifier();
  pType<> DEnum();
  pType<> DStruct();
  std::string DDirectDeclaratorImpl(std::stack<pType<PointerType>>& dest);
  ast::pNode<ast::DeclarationNode> DDeclarator(pType<> ctype, ast::StorageClass storage);
  void DInitDeclaratorList(std::vector<ast::pNode<ast::DeclarationNode>>& dest);
  bool IsDeclarationSpecifier(int after = 0);

  ast::pNode<ast::StatNode> SStatement();
  ast::pNode<ast::CompoundNode> SCompound();

  ast::pNode<ast::FunctionDefinitionNode> ExternalDeclaration(std::vector<ast::pNode<ast::DeclarationNode>>& dest);

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
  std::vector<ast::pNode<ast::DeclNode>> declarations{};
};

}
