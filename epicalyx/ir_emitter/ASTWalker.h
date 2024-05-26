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

  // public access for making the program
  void Visit(const ast::DeclarationNode& decl) final;
  void Visit(const ast::FunctionDefinitionNode& decl) final;

private:
  enum class State {
    Empty,  // top level statement
    Assign,
    Address, 
    Read,
    ConditionalBranch,
  };

  union StateData {
    // assign
    struct {
      var_index_t var;
    };

    // conditional branch
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

  // block updates within expressions, to prevent handling them twice
  // for example in 0186-dec_ary.c:L13:
  //  c = nodes[--d].index++;
  // we must prevent --d from happening twice, when reading and when 
  // writing back the value for nodes[d - 1].index
  //
  // The idea is that, whenever we encounter an update (pre OR post), we
  // block post-updates, read the expression, compute the new value
  // block pre-updates and write back the expression
  // this way, every update is handled exactly once.
  // In the example: --d is handled when nodes[--d].index is read,
  // and ....index++ is handled on writeback, where --d is skipped
  u32 block_pre_update = 0;
  u32 block_post_update = 0;

  // destinations on break / continue
  std::stack<block_label_t> break_stack{};
  std::stack<block_label_t> continue_stack{};

  // select statement to add case nodes to
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

  void VisitGlobalSymbol(const cotyl::CString& symbol);

  // add local variable
  std::pair<var_index_t, LocalData> AddLocal(cotyl::CString&& name, const type::AnyType& type, std::optional<var_index_t> arg_index = {});

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

  void Visit(const ast::IdentifierNode& decl) final;

  void Visit(const ast::NumericalConstantNode<i8>& expr) final { ConstVisitImpl(expr); }
  void Visit(const ast::NumericalConstantNode<u8>& expr) final { ConstVisitImpl(expr); }
  void Visit(const ast::NumericalConstantNode<i16>& expr) final { ConstVisitImpl(expr); }
  void Visit(const ast::NumericalConstantNode<u16>& expr) final { ConstVisitImpl(expr); }
  void Visit(const ast::NumericalConstantNode<i32>& expr) final { ConstVisitImpl(expr); }
  void Visit(const ast::NumericalConstantNode<u32>& expr) final { ConstVisitImpl(expr); }
  void Visit(const ast::NumericalConstantNode<i64>& expr) final { ConstVisitImpl(expr); }
  void Visit(const ast::NumericalConstantNode<u64>& expr) final { ConstVisitImpl(expr); }
  void Visit(const ast::NumericalConstantNode<float>& expr) final { ConstVisitImpl(expr); }
  void Visit(const ast::NumericalConstantNode<double>& expr) final { ConstVisitImpl(expr); }
  template<typename T>
  void ConstVisitImpl(const ast::NumericalConstantNode<T>& expr);

  void Visit(const ast::StringConstantNode& expr) final;
  void Visit(const ast::ArrayAccessNode& expr) final;
  void Visit(const ast::FunctionCallNode& expr) final;
  void Visit(const ast::MemberAccessNode& expr) final;
  void Visit(const ast::TypeInitializerNode& expr) final;
  void Visit(const ast::PostFixNode& expr) final;
  void Visit(const ast::UnopNode& expr) final;
  void Visit(const ast::CastNode& expr) final;
  void Visit(const ast::BinopNode& expr) final;
  void Visit(const ast::TernaryNode& expr) final;
  void Visit(const ast::AssignmentNode& expr) final;
  void Visit(const ast::ExpressionListNode& expr) final;

  void Visit(const ast::EmptyNode& stat) final;
  void Visit(const ast::IfNode& stat) final;
  void Visit(const ast::WhileNode& stat) final;
  void Visit(const ast::DoWhileNode& stat) final;
  void Visit(const ast::ForNode& stat) final;
  void Visit(const ast::LabelNode& stat) final;
  void Visit(const ast::SwitchNode& stat) final;
  void Visit(const ast::CaseNode& stat) final;
  void Visit(const ast::DefaultNode& stat) final;
  void Visit(const ast::GotoNode& stat) final;
  void Visit(const ast::ReturnNode& stat) final;
  void Visit(const ast::BreakNode& stat) final;
  void Visit(const ast::ContinueNode& stat) final;
  void Visit(const ast::CompoundNode& stat) final;
};

}