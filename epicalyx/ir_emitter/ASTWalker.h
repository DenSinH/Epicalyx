#pragma once

#include "ast/NodeVisitor.h"
#include "calyx/Calyx.h"
#include "Emitter.h"
#include "Scope.h"

#include <stack>
#include <stack>

namespace epi {


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

  struct LocalData {
    calyx::Local* loc;
    pType<const CType> type;
  };

  std::stack<std::pair<State, StateData>> state{};
  std::stack<calyx::block_label_t> break_stack{};
  std::stack<calyx::block_label_t> continue_stack{};
  std::stack<calyx::Select*> select_stack{};
  // goto labels
  cotyl::unordered_map<std::string, calyx::block_label_t> local_labels{};

  cotyl::MapScope<std::string, LocalData> locals{};
  cotyl::unordered_map<std::string, pType<const CType>> symbol_types{};
  
  calyx::var_index_t current;
  Emitter& emitter;

  void AddGlobal(const std::string& symbol, const pType<const CType>& type) {
    symbol_types.emplace(symbol, type);
  }

  void NewFunction(const std::string& symbol, const pType<const CType>& type) {
    emitter.NewFunction(symbol);
    symbol_types.emplace(symbol, type);

    // state = {};
    // break_stack = {};
    // continue_stack = {};
    // select_stack = {};
    local_labels.clear();
    cotyl::Assert(locals.Depth() == 1, "Local scope is not empty at function start");
    locals.Clear();
  }

  void EndFunction() {
    cotyl::Assert(locals.Depth() == 1, "Local scope is not empty after function");
  }

  static calyx::Local::Type GetCalyxType(const pType<const CType>& type);

  calyx::var_index_t AddLocal(const std::string& name, const pType<const CType>& type, std::optional<calyx::var_index_t> arg_index = {}) {
    auto c_idx = emitter.c_counter++;
    size_t size = type->Sizeof();
    auto& loc = emitter.current_function->locals.emplace(c_idx, calyx::Local{GetCalyxType(type), c_idx, size, std::move(arg_index)}).first->second;
    locals.Set(name, LocalData{ &loc, type });
    return c_idx;
  }

  const pType<const CType>& GetSymbolType(const std::string& symbol) const {
    if (locals.Has(symbol)) {
      return locals.Get(symbol).type;
    }
    return symbol_types.at(symbol);
  }

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