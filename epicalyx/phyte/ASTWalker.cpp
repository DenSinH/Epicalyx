#include "ASTWalker.h"
#include "Emitter.h"
#include "calyx/Calyx.h"
#include "types/EpiCType.h"
#include "taxy/Declaration.h"
#include "taxy/Statement.h"
#include "taxy/Expression.h"


namespace epi::phyte {

namespace detail {

template<typename T> struct calyx_type;
template<> struct calyx_type<i32> { static constexpr auto value = calyx::Var::Type::I32; };
template<> struct calyx_type<u32> { static constexpr auto value = calyx::Var::Type::U32; };
template<> struct calyx_type<i64> { static constexpr auto value = calyx::Var::Type::I64; };
template<> struct calyx_type<u64> { static constexpr auto value = calyx::Var::Type::U64; };
template<> struct calyx_type<float> { static constexpr auto value = calyx::Var::Type::Float; };
template<> struct calyx_type<double> { static constexpr auto value = calyx::Var::Type::Double; };
template<> struct calyx_type<calyx::Pointer> { static constexpr auto value = calyx::Var::Type::Pointer; };
template<> struct calyx_type<calyx::Struct> { static constexpr auto value = calyx::Var::Type::Struct; };
template<typename T>
constexpr auto calyx_type_v = calyx_type<T>::value;

template<class Sig>
struct func_args;

template<class R, typename First, class...Args>
struct func_args<R(First, Args...)> {
  using type = std::tuple<Args...>;
};
template<class Sig>
using func_args_t = typename func_args<Sig>::type;


template<template<typename T> class emit>
struct EmitterTypeVisitor : TypeVisitor {
  using args_t = func_args_t<decltype(emit<i32>::emit_value)>;

  EmitterTypeVisitor(ASTWalker& walker, args_t args) :
      walker(walker), args(args) {

  }

  ASTWalker& walker;
  args_t args;

  template<typename T>
  void VisitValueImpl() {
    walker.current = std::apply([&](auto... _args){ return emit<T>::emit_value(walker, _args...); }, args);
  }

  void VisitPointerImpl(u64 stride) {
    walker.current = std::apply([&](auto... _args){ return emit<calyx::Pointer>::emit_pointer(walker, stride, _args...); }, args);
  }

  void Visit(const VoidType& type) final { throw std::runtime_error("Incomplete type"); }
  void Visit(const ValueType<i8>& type) final { VisitValueImpl<i8>(); }
  void Visit(const ValueType<u8>& type) final { VisitValueImpl<u8>(); }
  void Visit(const ValueType<i16>& type) final { VisitValueImpl<i16>(); }
  void Visit(const ValueType<u16>& type) final { VisitValueImpl<u16>(); }
  void Visit(const ValueType<i32>& type) final { VisitValueImpl<i32>(); }
  void Visit(const ValueType<u32>& type) final { VisitValueImpl<u32>(); }
  void Visit(const ValueType<i64>& type) final { VisitValueImpl<i64>(); }
  void Visit(const ValueType<u64>& type) final { VisitValueImpl<u64>(); }
  void Visit(const ValueType<float>& type) final { VisitValueImpl<float>(); }
  void Visit(const ValueType<double>& type) final { VisitValueImpl<double>(); }
  void Visit(const PointerType& type) final { VisitPointerImpl(type.Deref()->Sizeof()); }
  void Visit(const ArrayType& type) final { VisitPointerImpl(type.Deref()->Sizeof()); }
  void Visit(const FunctionType& type) final { VisitPointerImpl(type.Deref()->Sizeof()); }
  void Visit(const StructType& type) final { throw std::runtime_error("Unimplemented"); }
  void Visit(const UnionType& type) final { throw std::runtime_error("Unimplemented"); }
};

template<typename T>
struct LoadCVarEmitter {
  static calyx::var_index_t emit_value(ASTWalker& walker, calyx::var_index_t c_idx) {
    return walker.emitter.EmitExpr<calyx::IRLoadCVar<T>>({ calyx_type_v<calyx::calyx_upcast_t<T>> }, c_idx);
  }

  static calyx::var_index_t emit_pointer(ASTWalker& walker, u64 stride, calyx::var_index_t c_idx) {
    return walker.emitter.EmitExpr<calyx::IRLoadCVar<calyx::Pointer>>({ calyx::Var::Type::Pointer, stride }, c_idx);
  }
};


template<typename T>
struct StoreCVarEmitter {
  static calyx::var_index_t do_cast(ASTWalker& walker, calyx::Var::Type src_t, calyx::var_index_t src_idx) {
    if (!std::is_same_v<calyx::calyx_upcast_t<T>, T> || (calyx_type_v<calyx::calyx_upcast_t<T>> != src_t)) {
      using upcast = typename calyx::calyx_upcast_t<T>;
      switch (src_t) {
        case calyx::Var::Type::I32: return walker.emitter.EmitExpr<calyx::IRCast<T, i32>>({ calyx_type_v<upcast> }, src_idx);
        case calyx::Var::Type::U32: return walker.emitter.EmitExpr<calyx::IRCast<T, u32>>({ calyx_type_v<upcast> }, src_idx);
        case calyx::Var::Type::I64: return walker.emitter.EmitExpr<calyx::IRCast<T, i64>>({ calyx_type_v<upcast> }, src_idx);
        case calyx::Var::Type::U64: return walker.emitter.EmitExpr<calyx::IRCast<T, u64>>({ calyx_type_v<upcast> }, src_idx);
        case calyx::Var::Type::Float: return walker.emitter.EmitExpr<calyx::IRCast<T, float>>({ calyx_type_v<upcast> }, src_idx);
        case calyx::Var::Type::Double: return walker.emitter.EmitExpr<calyx::IRCast<T, double>>({ calyx_type_v<upcast> }, src_idx);
        case calyx::Var::Type::Pointer: return walker.emitter.EmitExpr<calyx::IRCast<T, calyx::Pointer>>({ calyx_type_v<upcast> }, src_idx);
        default:
          return 0;
      }
    }
    return 0;
  }

  static calyx::var_index_t emit_value(ASTWalker& walker, calyx::var_index_t c_idx, calyx::var_index_t src) {
    calyx::var_index_t cast = do_cast(walker, walker.emitter.vars[src].type, src);
    cast = cast ? cast : src;
    return walker.emitter.EmitExpr<calyx::IRStoreCVar<T>>({ calyx_type_v<calyx::calyx_upcast_t<T>> }, c_idx, cast);
  }

  static calyx::var_index_t emit_pointer(ASTWalker& walker, u64 stride, calyx::var_index_t c_idx, calyx::var_index_t src) {
    calyx::var_index_t cast = do_cast(walker, walker.emitter.vars[src].type, src);
    cast = cast ? cast : src;
    return walker.emitter.EmitExpr<calyx::IRStoreCVar<T>>({ calyx::Var::Type::Pointer, stride }, c_idx, cast);
  }
};

template<typename T>
struct LoadCVarAddrEmitter {
  static calyx::var_index_t emit_value(ASTWalker& walker, calyx::var_index_t c_idx) {
    return walker.emitter.EmitExpr<calyx::IRLoadCVarAddr>({ calyx::Var::Type::Pointer, sizeof(T) }, c_idx);
  }

  static calyx::var_index_t emit_pointer(ASTWalker& walker, [[maybe_unused]] u64 stride, calyx::var_index_t c_idx) {
    return walker.emitter.EmitExpr<calyx::IRLoadCVarAddr>({ calyx::Var::Type::Pointer, sizeof(u64) }, c_idx);
  }
};


}

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
    c_types.Set(decl.name, decl.type);
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

template<typename T>
void EmitLoadCVar(calyx::var_index_t idx) {

}

void ASTWalker::Visit(Identifier& decl) {
  // after the AST, the only identifiers left are C variables
  auto type = c_types.Get(decl.name);
  auto cvar = variables.Get(decl.name);
  switch (state.top().first) {
    case State::Read: {
      if (type->IsArray()) {
        current = emitter.EmitExpr<calyx::IRLoadCVarAddr>({ calyx::Var::Type::Pointer, type->Deref()->Sizeof() }, cvar.idx);
      }
      else {
        auto visitor = detail::EmitterTypeVisitor<detail::LoadCVarEmitter>(*this, { cvar.idx });
        type->Visit(visitor);
      }
      break;
    }
    case State::Assign: {
      auto visitor = detail::EmitterTypeVisitor<detail::StoreCVarEmitter>(
              *this, { cvar.idx, state.top().second }
      );
      type->Visit(visitor);
      break;
    }
    case State::Address: {
      // we can't get the address of a variable that is not on the stack
      variables.Get(decl.name).loc = calyx::CVar::Location::Stack;
      auto visitor = detail::EmitterTypeVisitor<detail::LoadCVarAddrEmitter>(*this, { cvar.idx });
      type->Visit(visitor);
      break;
    }
    default: {
      throw std::runtime_error("Unimplemented");
    }
  }
}

void ASTWalker::Visit(NumericalConstant<i8>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<i32>>({ detail::calyx_type_v<calyx::calyx_upcast_t<i8>> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<u8>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<u32>>({ detail::calyx_type_v<calyx::calyx_upcast_t<u8>> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<i16>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<i32>>({ detail::calyx_type_v<calyx::calyx_upcast_t<i16>> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<u16>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<u32>>({ detail::calyx_type_v<calyx::calyx_upcast_t<u16>> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<i32>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<i32>>({ detail::calyx_type_v<i32> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<u32>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<u32>>({ detail::calyx_type_v<u32> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<i64>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<i64>>({ detail::calyx_type_v<i64> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<u64>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<u64>>({ detail::calyx_type_v<u64> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<float>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<float>>({ detail::calyx_type_v<float> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<double>& expr) {
  current = emitter.EmitExpr<calyx::IRImm<double>>({ detail::calyx_type_v<double> }, expr.value);
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
