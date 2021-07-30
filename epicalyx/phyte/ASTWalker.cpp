#include "ASTWalker.h"
#include "Emitter.h"
#include "calyx/Calyx.h"
#include "types/EpiCType.h"
#include "taxy/Declaration.h"
#include "taxy/Statement.h"
#include "taxy/Expression.h"


namespace epi::phyte {

template<template<typename T> class IROp, typename... Args>
struct EmitterTypeVisitor : TypeVisitor {

  EmitterTypeVisitor(ASTWalker& walker, const Args&... args) :
      walker(walker), args(std::make_tuple(args...)) {

  }

  ASTWalker& walker;
  std::tuple<Args...> args;

  template<typename T>
  void VisitImpl(const ValueType<T>& type) {
    walker.current = std::apply([&](Args... _args){ return walker.emitter.EmitExpr<IROp<T>>(_args...); }, args);
  }

  void Visit(const VoidType& type) final { throw std::runtime_error("Incomplete type"); }
  void Visit(const ValueType<i8>& type) final { VisitImpl(type); }
  void Visit(const ValueType<u8>& type) final { VisitImpl(type); }
  void Visit(const ValueType<i16>& type) final { VisitImpl(type); }
  void Visit(const ValueType<u16>& type) final { VisitImpl(type); }
  void Visit(const ValueType<i32>& type) final { VisitImpl(type); }
  void Visit(const ValueType<u32>& type) final { VisitImpl(type); }
  void Visit(const ValueType<i64>& type) final { VisitImpl(type); }
  void Visit(const ValueType<u64>& type) final { VisitImpl(type); }
  void Visit(const ValueType<float>& type) final { VisitImpl(type); }
  void Visit(const ValueType<double>& type) final { VisitImpl(type); }
  void Visit(const PointerType& type) final { walker.current = std::apply([&](Args... _args){ return walker.emitter.EmitExpr<IROp<u64>>(_args...); }, args); }
  void Visit(const ArrayType& type) final { walker.current = std::apply([&](Args... _args){ return walker.emitter.EmitExpr<IROp<u64>>(_args...); }, args); }
  void Visit(const FunctionType& type) final { walker.current = std::apply([&](Args... _args){ return walker.emitter.EmitExpr<IROp<u64>>(_args...); }, args); }
  void Visit(const StructType& type) final { throw std::runtime_error("Unimplemented"); }
  void Visit(const UnionType& type) final { throw std::runtime_error("Unimplemented"); }
};


void ASTWalker::Visit(epi::taxy::Declaration& decl) {
  if (variables.Depth() == 1) {
    // global symbols
  }
  else {
    auto c_idx = emitter.c_counter++;
    u64 size = decl.type->Sizeof();
    variables.Set(decl.name, calyx::CVar{
            c_idx, calyx::CVar::Location::Either, size
    });
    c_variables.Set(decl.name, decl.type);
    emitter.Emit<calyx::IRAllocateCVar>(c_idx, size);

    if (decl.value.has_value()) {
      throw std::runtime_error("Unimplemented");
    }
  }
}

void ASTWalker::Visit(epi::taxy::FunctionDefinition& decl) {
  // same as normal compound statement besides arguments
  variables.NewLayer();
  // todo: arguments etc
  for (const auto& node : decl.body->stats) {
    node->Visit(*this);
  }
  for (const auto& var : variables.Top()) {
    emitter.Emit<calyx::IRDeallocateCVar>(var.second.idx, var.second.size);
  }
  variables.PopLayer();
}

void ASTWalker::Visit(Identifier& decl) {
  // after the AST, the only identifiers left are C variables
  auto type = c_variables.Get(decl.name);
  auto cvar = variables.Get(decl.name);
  switch (state.top().first) {
    case State::Read: {
      if (type->IsArray()) {
        current = emitter.EmitExpr<calyx::IRLoadCVarAddr>(cvar.idx);
      }
      else {
        auto visitor = EmitterTypeVisitor<calyx::IRLoadCVar, calyx::var_index_t>(*this, cvar.idx);
        type->Visit(visitor);
      }
      break;
    }
    case State::Assign: {
      auto visitor = EmitterTypeVisitor<calyx::IRStoreCVar, calyx::var_index_t, calyx::var_index_t>(*this, cvar.idx, state.top().second);
      type->Visit(visitor);
      break;
    }
    case State::Address: {
      // we can't get the address of a variable that is not on the stack
      variables.Get(decl.name).loc = calyx::CVar::Location::Stack;
      current = emitter.EmitExpr<calyx::IRLoadCVarAddr>(cvar.idx);
      break;
    }
    default: {
      throw std::runtime_error("Unimplemented");
    }
  }
}

void ASTWalker::Visit(NumericalConstant<i8>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<i32>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<u8>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<u32>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<i16>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<i32>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<u16>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<u32>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<i32>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<i32>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<u32>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<u32>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<i64>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<i64>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<u64>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<u64>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<float>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<float>>(expr.value);
}

void ASTWalker::Visit(NumericalConstant<double>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<double>>(expr.value);
}

void ASTWalker::Visit(StringConstant& expr) {
  // load from rodata
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(ArrayAccess& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(FunctionCall& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(MemberAccess& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(TypeInitializer& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(PostFix& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Unary& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Cast& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Binop& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Ternary& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Assignment& expr) {
  state.emplace(State::Read, 0);
  expr.right->Visit(*this);
  state.pop();
  // current now holds the expression id that we want to assign with
  state.emplace(State::Assign, current);
  expr.left->Visit(*this);
  state.pop();
}

void ASTWalker::Visit(Empty& stat) {

}

void ASTWalker::Visit(If& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(While& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(DoWhile& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(For& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Label& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Switch& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Case& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Default& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Goto& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Return& stat) {
  if (stat.expr) {
    stat.expr->Visit(*this);
    emitter.Emit<calyx::IRReturn>(current);
  }
  else {
    emitter.Emit<calyx::IRReturn>(0);
  }
}

void ASTWalker::Visit(Break& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Continue& stat) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Compound& stat) {
  variables.NewLayer();
  for (const auto& node : stat.stats) {
    node->Visit(*this);
  }
  for (const auto& var : variables.Top()) {
    emitter.Emit<calyx::IRDeallocateCVar>(var.second.idx, var.second.size);
  }
  variables.PopLayer();
}


}
