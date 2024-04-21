#pragma once

#include "ConstParser.h"
#include "Scope.h"
#include "Containers.h"
#include "types/TypeFwd.h"
#include "ast/Initializer.h"

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
  type::AnyType ETypeName();
  ast::pExpr ECast() final;
  ast::pExpr EAssignment() final;
  ast::pExpr EExpression();
  ast::Initializer EInitializer();
  ast::InitializerList EInitializerList();
  type::AnyType ResolveIdentifierType(const cotyl::CString& name) const;
  void EExpressionList(cotyl::vector<ast::pExpr>& dest);

  void DStaticAssert();
  std::pair<type::AnyType, ast::StorageClass> DSpecifier();
  type::AnyType DEnum();
  type::AnyType DStruct();
  cotyl::CString DDirectDeclaratorImpl(std::stack<std::unique_ptr<type::AnyPointerType>>& dest);
  ast::pNode<ast::DeclarationNode> DDeclarator(type::AnyType ctype, ast::StorageClass storage);
  void DInitDeclaratorList(cotyl::vector<ast::pNode<ast::DeclarationNode>>& dest);
  bool IsDeclarationSpecifier(int after = 0);

  ast::pNode<ast::StatNode> SStatement();
  ast::pNode<ast::CompoundNode> SCompound();

  ast::pNode<ast::FunctionDefinitionNode> ExternalDeclaration(cotyl::vector<ast::pNode<ast::DeclarationNode>>& dest);

  void PushScope();
  void PopScope();

  void Parse();
  void Data();

  enum class Loop {
    For, While, DoWhile
  };

  cotyl::MapScope<cotyl::CString, enum_type> enum_values{};
  cotyl::SetScope<cotyl::CString> enums{};
  cotyl::MapScope<cotyl::CString, type::AnyType> typedefs{};
  cotyl::MapScope<cotyl::CString, type::StructType> structdefs{};
  cotyl::MapScope<cotyl::CString, type::UnionType> uniondefs{};

  const type::AnyType* function_return{};

  std::deque<Loop> loop_scope{};
  cotyl::SetScope<i64> case_scope{};
  cotyl::unordered_set<cotyl::CString> labels{};
  cotyl::unordered_set<cotyl::CString> unresolved_labels{};
  cotyl::MapScope<cotyl::CString, type::AnyType> variables{};

  // external results
  cotyl::vector<ast::pNode<ast::DeclNode>> declarations{};
};

}
