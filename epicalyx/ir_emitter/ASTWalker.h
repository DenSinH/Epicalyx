#pragma once

#include "ast/NodeVisitor.h"
#include "types/AnyType.h"
#include "calyx/CalyxFwd.h"
#include "Emitter.h"
#include "Scope.h"
#include "Helpers.h"

#include <stack>

namespace epi {


struct ASTWalker : public ast::NodeVisitor {
  ASTWalker(Emitter& emitter) :
      emitter(emitter) {
    state.push({State::Empty, {}});
  }

  // public access for helpers
  var_index_t current;
  Emitter& emitter;

private:
  enum class State {
    Empty, Assign, Address, Read, ConditionalBranch,
  };

  union StateData {
    struct {
      var_index_t var;
    };

    struct {
      var_index_t true_block;
      var_index_t false_block;
    };
  };

  struct LocalData {
    calyx::Local* loc;
    type::AnyType type;
  };

  std::stack<std::pair<State, StateData>> state{};
  std::stack<block_label_t> break_stack{};
  std::stack<block_label_t> continue_stack{};
  std::stack<calyx::Select*> select_stack{};
  // goto labels
  cotyl::unordered_map<cotyl::CString, block_label_t> local_labels{};

  cotyl::MapScope<cotyl::CString, LocalData> locals{};
  cotyl::unordered_map<cotyl::CString, type::AnyType> symbol_types{};

  // helper for conditional branching
  void EmitConditionalBranchForCurrent();

  // debug function that asserts a valid function start / end state
  void AssertClearState();

  // start new function
  void NewFunction(cotyl::CString&& symbol, const type::AnyType& type);

  // end function
  void EndFunction();

  // add global variable
  void AddGlobal(const cotyl::CString& symbol, const type::AnyType& type);
  // add local variable
  var_index_t AddLocal(cotyl::CString&& name, const type::AnyType& type, std::optional<var_index_t> arg_index = {});

  // get symbol type (local or global, depending on scope)
  const type::AnyType& GetSymbolType(const cotyl::CString& symbol) const;

  const ast::FunctionDefinitionNode* function = nullptr;

  template<template<typename T> class Op, typename... Args>
  void EmitIntegralExpr(Emitter::Var::Type type, Args... args);
  template<template<typename T> class Op, typename... Args>
  void EmitArithExpr(Emitter::Var::Type type, Args... args);
  template<template<typename T> class Op, typename... Args>
  void EmitPointerIntegralExpr(Emitter::Var::Type type, u64 stride, Args... args);
  template<template<typename T> class Op, typename... Args>
  void EmitPointerExpr(Emitter::Var::Type type, u64 stride, Args... args);
  template<template<typename T> class Op, typename... Args>
  void EmitBranch(Emitter::Var::Type type, Args... args);
  template<template<typename T> class Op, typename... Args>
  void EmitCompare(Emitter::Var::Type type, Args... args);

  struct BinopCastResult {
    Emitter::Var var;
    var_index_t left;
    var_index_t right;
  };
  BinopCastResult BinopCastHelper(var_index_t left, var_index_t right);
  void BinopHelper(var_index_t left, calyx::BinopType op, var_index_t right);

  void Visit(ast::DeclarationNode& decl) final;
  void Visit(ast::FunctionDefinitionNode& decl) final;
  void Visit(ast::IdentifierNode& decl) final;

  void Visit(ast::NumericalConstantNode<i8>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstantNode<u8>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstantNode<i16>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstantNode<u16>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstantNode<i32>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstantNode<u32>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstantNode<i64>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstantNode<u64>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstantNode<float>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstantNode<double>& expr) final { ConstVisitImpl(expr); }
  template<typename T>
  void ConstVisitImpl(ast::NumericalConstantNode<T>& expr);

  void Visit(ast::StringConstantNode& expr) final;
  void Visit(ast::ArrayAccessNode& expr) final;
  void Visit(ast::FunctionCallNode& expr) final;
  void Visit(ast::MemberAccessNode& expr) final;
  void Visit(ast::TypeInitializerNode& expr) final;
  void Visit(ast::PostFixNode& expr) final;
  void Visit(ast::UnopNode& expr) final;
  void Visit(ast::CastNode& expr) final;
  void Visit(ast::BinopNode& expr) final;
  void Visit(ast::TernaryNode& expr) final;
  void Visit(ast::AssignmentNode& expr) final;

  void Visit(ast::EmptyNode& stat) final;
  void Visit(ast::IfNode& stat) final;
  void Visit(ast::WhileNode& stat) final;
  void Visit(ast::DoWhileNode& stat) final;
  void Visit(ast::ForNode& stat) final;
  void Visit(ast::LabelNode& stat) final;
  void Visit(ast::SwitchNode& stat) final;
  void Visit(ast::CaseNode& stat) final;
  void Visit(ast::DefaultNode& stat) final;
  void Visit(ast::GotoNode& stat) final;
  void Visit(ast::ReturnNode& stat) final;
  void Visit(ast::BreakNode& stat) final;
  void Visit(ast::ContinueNode& stat) final;
  void Visit(ast::CompoundNode& stat) final;
};

}