#pragma once

#include "ConstParser.h"
#include "Exceptions.h"
#include "Scope.h"
#include "Containers.h"
#include "types/AnyType.h"
#include "ast/Initializer.h"
#include "ast/Declaration.h"  // FunctionDef for vector destructor
#include "ast/Statement.h"  // compoundnode for FunctionDef
#include "Variant.h"

#include <stack>

namespace epi {


struct Parser final : public ConstParser {
  using ConstParser::ConstParser;

  using enum_type = i32;

  ast::pExpr EExpression();

  // needs to be available publicly for shorthand binop parsing
  ast::pExpr ECast() final;

  // external results
  void Parse();
  void Data();

  cotyl::vector<ast::DeclarationNode> declarations{};
  cotyl::vector<ast::FunctionDefinitionNode> functions{};

  // needed public for unwinding function
  using any_pointer_t = cotyl::Variant<
    type::AnyPointerType,
    type::PointerType,
    type::ArrayType,
    type::FunctionType
  >;
private:
  ast::pExpr EPrimary() final;
  ast::pExpr EPostfix();
  ast::pExpr EUnary();
  ast::pExpr EAssignment() final;
  ast::pExpr EExpressionList();

  type::AnyType ETypeName();
  ast::Initializer EInitializer();
  ast::InitializerList EInitializerList();

  type::AnyType ResolveIdentifierType(const cotyl::CString& name) const;

  void DStaticAssert();
  std::pair<type::AnyType, ast::StorageClass> DSpecifier();
  type::AnyType DEnum();
  type::AnyType DStruct();
  cotyl::CString DDirectDeclaratorImpl(std::stack<any_pointer_t>& dest);
  ast::DeclarationNode DDeclarator(type::AnyType ctype, ast::StorageClass storage);
  void DInitDeclaratorList(cotyl::vector<ast::DeclarationNode>& dest);
  void StoreDeclaration(ast::DeclarationNode&& decl, cotyl::vector<ast::DeclarationNode>& dest);
  bool IsDeclarationSpecifier(int after = 0);
  void RecordDeclaration(const cotyl::CString& name, type::AnyType& type);

  ast::pNode<ast::StatNode> SStatement();
  ast::pNode<ast::CompoundNode> SCompound();

  void ExternalDeclaration();

  void PushScope();
  void PopScope();

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
};

}
