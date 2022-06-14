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

  const ast::FunctionDefinition* function = nullptr;

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

  void Visit(ast::Declaration& decl) final;
  void Visit(ast::FunctionDefinition& decl) final;
  void Visit(ast::Identifier& decl) final;

  void Visit(ast::NumericalConstant<i8>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstant<u8>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstant<i16>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstant<u16>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstant<i32>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstant<u32>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstant<i64>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstant<u64>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstant<float>& expr) final { ConstVisitImpl(expr); }
  void Visit(ast::NumericalConstant<double>& expr) final { ConstVisitImpl(expr); }
  template<typename T>
  void ConstVisitImpl(ast::NumericalConstant<T>& expr);

  void Visit(ast::StringConstant& expr) final;
  void Visit(ast::ArrayAccess& expr) final;
  void Visit(ast::FunctionCall& expr) final;
  void Visit(ast::MemberAccess& expr) final;
  void Visit(ast::TypeInitializer& expr) final;
  void Visit(ast::PostFix& expr) final;
  void Visit(ast::Unary& expr) final;
  void Visit(ast::Cast& expr) final;
  void Visit(ast::Binop& expr) final;
  void Visit(ast::Ternary& expr) final;
  void Visit(ast::Assignment& expr) final;

  void Visit(ast::Empty& stat) final;
  void Visit(ast::If& stat) final;
  void Visit(ast::While& stat) final;
  void Visit(ast::DoWhile& stat) final;
  void Visit(ast::For& stat) final;
  void Visit(ast::Label& stat) final;
  void Visit(ast::Switch& stat) final;
  void Visit(ast::Case& stat) final;
  void Visit(ast::Default& stat) final;
  void Visit(ast::Goto& stat) final;
  void Visit(ast::Return& stat) final;
  void Visit(ast::Break& stat) final;
  void Visit(ast::Continue& stat) final;
  void Visit(ast::Compound& stat) final;
};

}