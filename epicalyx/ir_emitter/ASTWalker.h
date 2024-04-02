#pragma once

#include "ast/NodeVisitor.h"
#include "calyx/Calyx.h"
#include "Scope.h"

#include <stack>

namespace epi {

struct Emitter;


struct ASTWalker : public ast::NodeVisitor {
  ASTWalker(Emitter& emitter) :
      emitter(emitter) {
    state.push({State::Empty, {}});
  }

  enum class State {
    Empty, Assign, Address, Read, ConditionalBranch,
  };

  union StateData {
    struct {
      calyx::var_index_t var;
    };

    struct {
      calyx::var_index_t true_block;
      calyx::var_index_t false_block;
    };
  };

  struct Local {
    calyx::var_index_t idx;
    u64 size;
  };

  std::stack<std::pair<State, StateData>> state{};
  std::stack<calyx::block_label_t> break_stack{};
  std::stack<calyx::block_label_t> continue_stack{};
  std::stack<calyx::Select*> select_stack{};
  cotyl::unordered_map<std::string, calyx::block_label_t> local_labels{};

  cotyl::MapScope<std::string, Local> locals{};
  cotyl::MapScope<std::string, pType<const CType>> local_types{};

  calyx::var_index_t current;
  Emitter& emitter;

  const ast::FunctionDefinitionNode* function = nullptr;

  template<template<typename T> class Op, typename... Args>
  void EmitIntegralExpr(calyx::Var::Type type, Args... args);
  template<template<typename T> class Op, typename... Args>
  void EmitArithExpr(calyx::Var::Type type, Args... args);
  template<template<typename T> class Op, typename... Args>
  void EmitPointerIntegralExpr(calyx::Var::Type type, u64 stride, Args... args);
  template<template<typename T> class Op, typename... Args>
  void EmitPointerExpr(calyx::Var::Type type, u64 stride, Args... args);
  template<template<typename T> class Op, typename... Args>
  void EmitBranch(calyx::Var::Type type, Args... args);
  template<template<typename T> class Op, typename... Args>
  void EmitCompare(calyx::Var::Type type, Args... args);

  struct BinopCastResult {
    calyx::Var var;
    calyx::var_index_t left;
    calyx::var_index_t right;
  };
  BinopCastResult BinopCastHelper(calyx::var_index_t left, calyx::var_index_t right);
  void BinopHelper(calyx::var_index_t left, calyx::BinopType op, calyx::var_index_t right);

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