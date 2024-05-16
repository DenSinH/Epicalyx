#pragma once

#include "ExpressionParser.h"
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


struct Parser final : public ExpressionParser {
  using ExpressionParser::ExpressionParser;

  using enum_type = i32;

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
  ast::pExpr EBinopBase() final;
  ast::pExpr ResolveIdentifier(cotyl::CString&& name) const final;
  ast::pExpr EPostfix();
  ast::pExpr EUnary();
  ast::pExpr ECast();
  ast::pExpr EAssignment() final;

  type::AnyType ETypeName();
  ast::Initializer EInitializer();
  ast::InitializerList EInitializerList();
  
  void DStaticAssert();
  std::pair<type::AnyType, ast::StorageClass> DSpecifier();
  type::AnyType DEnum();
  type::AnyType DStruct();
  cotyl::CString DDirectDeclaratorImpl(std::stack<any_pointer_t>& dest);
  ast::DeclarationNode DDeclarator(type::AnyType ctype, ast::StorageClass storage);
  void CompleteForwardDecl(type::AnyType& fwd_decl) const;
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
  const char* function_symbol{};

  std::deque<Loop> loop_scope{};
  cotyl::SetScope<i64> case_scope{};
  cotyl::unordered_set<cotyl::CString> labels{};
  cotyl::unordered_set<cotyl::CString> unresolved_labels{};
  cotyl::MapScope<cotyl::CString, type::AnyType> variables{};
};

}
