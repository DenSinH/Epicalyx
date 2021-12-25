#pragma once

#include "taxy/NodeVisitor.h"
#include "calyx/Calyx.h"
#include "Scope.h"

#include <stack>

namespace epi::phyte {

struct Emitter;

using namespace taxy;

struct ASTWalker : public taxy::NodeVisitor {
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

  std::stack<std::pair<State, StateData>> state{};
  std::stack<calyx::block_label_t> break_stack{};
  std::stack<calyx::block_label_t> continue_stack{};
  std::stack<calyx::Select*> select_stack{};

  cotyl::MapScope<std::string, calyx::Local> variables{};
  cotyl::MapScope<std::string, pType<const CType>> c_types{};

  calyx::var_index_t current;
  Emitter& emitter;

  const FunctionDefinition* function = nullptr;

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

  void Visit(Declaration& decl) final;
  void Visit(FunctionDefinition& decl) final;
  void Visit(Identifier& decl) final;

  void Visit(NumericalConstant<i8>& expr) final { ConstVisitImpl(expr); }
  void Visit(NumericalConstant<u8>& expr) final { ConstVisitImpl(expr); }
  void Visit(NumericalConstant<i16>& expr) final { ConstVisitImpl(expr); }
  void Visit(NumericalConstant<u16>& expr) final { ConstVisitImpl(expr); }
  void Visit(NumericalConstant<i32>& expr) final { ConstVisitImpl(expr); }
  void Visit(NumericalConstant<u32>& expr) final { ConstVisitImpl(expr); }
  void Visit(NumericalConstant<i64>& expr) final { ConstVisitImpl(expr); }
  void Visit(NumericalConstant<u64>& expr) final { ConstVisitImpl(expr); }
  void Visit(NumericalConstant<float>& expr) final { ConstVisitImpl(expr); }
  void Visit(NumericalConstant<double>& expr) final { ConstVisitImpl(expr); }
  template<typename T>
  void ConstVisitImpl(NumericalConstant<T>& expr);

  void Visit(StringConstant& expr) final;
  void Visit(ArrayAccess& expr) final;
  void Visit(FunctionCall& expr) final;
  void Visit(MemberAccess& expr) final;
  void Visit(TypeInitializer& expr) final;
  void Visit(PostFix& expr) final;
  void Visit(Unary& expr) final;
  void Visit(Cast& expr) final;
  void Visit(Binop& expr) final;
  void Visit(Ternary& expr) final;
  void Visit(Assignment& expr) final;

  void Visit(Empty& stat) final;
  void Visit(If& stat) final;
  void Visit(While& stat) final;
  void Visit(DoWhile& stat) final;
  void Visit(For& stat) final;
  void Visit(Label& stat) final;
  void Visit(Switch& stat) final;
  void Visit(Case& stat) final;
  void Visit(Default& stat) final;
  void Visit(Goto& stat) final;
  void Visit(Return& stat) final;
  void Visit(Break& stat) final;
  void Visit(Continue& stat) final;
  void Visit(Compound& stat) final;
};

}