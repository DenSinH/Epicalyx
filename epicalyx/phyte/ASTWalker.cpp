#include "ASTWalker.h"
#include "Emitter.h"
#include "calyx/Calyx.h"
#include "types/EpiCType.h"
#include "taxy/Declaration.h"
#include "taxy/Statement.h"
#include "taxy/Expression.h"

#include "Assert.h"


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
struct func;

template<class R, typename First, class...Args>
struct func<R(First, Args...)> {
  using type = std::tuple<Args...>;
  using return_t = R;
};
template<class Sig>
using func_args_t = typename func<Sig>::type;
template<class Sig>
using func_return_t = typename func<Sig>::return_t;


template<template<typename T> class emit>
struct EmitterTypeVisitor : TypeVisitor {
  using args_t = func_args_t<decltype(emit<i32>::emit_value)>;
  using return_t = func_return_t<decltype(emit<i32>::emit_value)>;

  EmitterTypeVisitor(ASTWalker& walker, args_t args) :
      walker(walker), args(args) {

  }

  ASTWalker& walker;
  args_t args;

  template<typename T>
  void VisitValueImpl() {
    if constexpr(std::is_same_v<return_t, calyx::var_index_t>) {
      walker.current = std::apply([&](auto... _args){ return emit<T>::emit_value(walker, _args...); }, args);
    }
    else if constexpr(std::is_same_v<return_t , void>) {
      std::apply([&](auto... _args){ return emit<T>::emit_value(walker, _args...); }, args);
    }
    else {
      []<bool flag = false> { static_assert(flag); }();
    }
  }

  void VisitPointerImpl(u64 stride) {
    if constexpr(std::is_same_v<return_t, calyx::var_index_t>) {
      walker.current = std::apply([&](auto... _args){ return emit<calyx::Pointer>::emit_pointer(walker, stride, _args...); }, args);
    }
    else if constexpr(std::is_same_v<return_t , void>) {
      std::apply([&](auto... _args){ return emit<calyx::Pointer>::emit_pointer(walker, stride, _args...); }, args);
    }
    else {
      []<bool flag = false> { static_assert(flag); }();
    }
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
    return walker.emitter.EmitExpr<calyx::LoadCVar<T>>({ calyx_type_v<calyx::calyx_upcast_t<T>> }, c_idx);
  }

  static calyx::var_index_t emit_pointer(ASTWalker& walker, u64 stride, calyx::var_index_t c_idx) {
    return walker.emitter.EmitExpr<calyx::LoadCVar<calyx::Pointer>>({ calyx::Var::Type::Pointer, stride }, c_idx);
  }
};


template<typename T>
struct CastToEmitter {
  static calyx::var_index_t do_cast(ASTWalker& walker, calyx::Var::Type src_t, calyx::var_index_t src_idx) {
    if (!std::is_same_v<calyx::calyx_upcast_t<T>, T> || (calyx_type_v<calyx::calyx_upcast_t<T>> != src_t)) {
      using upcast = typename calyx::calyx_upcast_t<T>;
      switch (src_t) {
        case calyx::Var::Type::I32: return walker.emitter.EmitExpr<calyx::Cast<T, i32>>({ calyx_type_v<upcast> }, src_idx);
        case calyx::Var::Type::U32: return walker.emitter.EmitExpr<calyx::Cast<T, u32>>({ calyx_type_v<upcast> }, src_idx);
        case calyx::Var::Type::I64: return walker.emitter.EmitExpr<calyx::Cast<T, i64>>({ calyx_type_v<upcast> }, src_idx);
        case calyx::Var::Type::U64: return walker.emitter.EmitExpr<calyx::Cast<T, u64>>({ calyx_type_v<upcast> }, src_idx);
        case calyx::Var::Type::Float: return walker.emitter.EmitExpr<calyx::Cast<T, float>>({ calyx_type_v<upcast> }, src_idx);
        case calyx::Var::Type::Double: return walker.emitter.EmitExpr<calyx::Cast<T, double>>({ calyx_type_v<upcast> }, src_idx);
        case calyx::Var::Type::Pointer: return walker.emitter.EmitExpr<calyx::Cast<T, calyx::Pointer>>({ calyx_type_v<upcast> }, src_idx);
        default:
          return 0;
      }
    }
    return 0;
  }

  static calyx::var_index_t emit_value(ASTWalker& walker, calyx::var_index_t src) {
    calyx::var_index_t cast = do_cast(walker, walker.emitter.vars[src].type, src);
    return cast ? cast : src;
  }

  static calyx::var_index_t emit_pointer(ASTWalker& walker, u64 stride, calyx::var_index_t src) {
    throw std::runtime_error("Unimplemented: cast to pointer");
  }
};

template<typename T>
struct LoadCVarAddrEmitter {
  static calyx::var_index_t emit_value(ASTWalker& walker, calyx::var_index_t c_idx) {
    return walker.emitter.EmitExpr<calyx::LoadCVarAddr>({ calyx::Var::Type::Pointer, sizeof(T) }, c_idx);
  }

  static calyx::var_index_t emit_pointer(ASTWalker& walker, [[maybe_unused]] u64 stride, calyx::var_index_t c_idx) {
    return walker.emitter.EmitExpr<calyx::LoadCVarAddr>({ calyx::Var::Type::Pointer, sizeof(u64) }, c_idx);
  }
};

template<typename T>
struct StoreCVarEmitter {
  static calyx::var_index_t emit_value(ASTWalker& walker, calyx::var_index_t c_idx, calyx::var_index_t src) {
    calyx::var_index_t cast = CastToEmitter<T>::emit_value(walker, src);
    return walker.emitter.EmitExpr<calyx::StoreCVar<T>>({ calyx_type_v<calyx::calyx_upcast_t<T>> }, c_idx, cast);
  }

  static calyx::var_index_t emit_pointer(ASTWalker& walker, u64 stride, calyx::var_index_t c_idx, calyx::var_index_t src) {
    calyx::var_index_t cast = CastToEmitter<T>::emit_pointer(walker, stride, src);
    return walker.emitter.EmitExpr<calyx::StoreCVar<T>>({ calyx::Var::Type::Pointer, stride }, c_idx, cast);
  }
};

template<typename T>
struct ReturnEmitter {
  static void emit_value(ASTWalker& walker, calyx::var_index_t src) {
    calyx::var_index_t cast = CastToEmitter<T>::emit_value(walker, src);
    walker.emitter.Emit<calyx::Return<calyx::calyx_upcast_t<T>>>(cast);
  }

  static void emit_pointer(ASTWalker& walker, u64 stride, calyx::var_index_t src) {
    calyx::var_index_t cast = CastToEmitter<T>::emit_pointer(walker, stride, src);
    walker.emitter.Emit<calyx::Return<T>>(cast);
  }
};

}

template<template<typename T> class Op, typename... Args>
void ASTWalker::EmitExpr(calyx::Var::Type type, Args... args) {
  switch (type) {
    case calyx::Var::Type::I32: current = emitter.EmitExpr<Op<i32>>({ type }, args...); break;
    case calyx::Var::Type::U32: current = emitter.EmitExpr<Op<u32>>({ type }, args...); break;
    case calyx::Var::Type::I64: current = emitter.EmitExpr<Op<i64>>({ type }, args...); break;
    case calyx::Var::Type::U64: current = emitter.EmitExpr<Op<u64>>({ type }, args...); break;
    case calyx::Var::Type::Float: current = emitter.EmitExpr<Op<float>>({ type }, args...); break;
    case calyx::Var::Type::Double: current = emitter.EmitExpr<Op<double>>({ type }, args...); break;
    case calyx::Var::Type::Pointer: current = emitter.EmitExpr<Op<calyx::Pointer>>({ type }, args...); break;
    case calyx::Var::Type::Struct: throw std::runtime_error("Unimplemented");
  }
}

template<template<typename T> class Op, typename... Args>
void ASTWalker::EmitArithExpr(calyx::Var::Type type, Args... args) {
  switch (type) {
    case calyx::Var::Type::I32: current = emitter.EmitExpr<Op<i32>>({ type }, args...); break;
    case calyx::Var::Type::U32: current = emitter.EmitExpr<Op<u32>>({ type }, args...); break;
    case calyx::Var::Type::I64: current = emitter.EmitExpr<Op<i64>>({ type }, args...); break;
    case calyx::Var::Type::U64: current = emitter.EmitExpr<Op<u64>>({ type }, args...); break;
    case calyx::Var::Type::Float: current = emitter.EmitExpr<Op<float>>({ type }, args...); break;
    case calyx::Var::Type::Double: current = emitter.EmitExpr<Op<double>>({ type }, args...); break;
    default:
      throw std::runtime_error("Bad type for arithmetic expression");
  }
}

void ASTWalker::BinopHelper(calyx::var_index_t left, calyx::BinopType op, calyx::var_index_t right) {
  auto left_t = emitter.vars[left].type;
  auto right_t = emitter.vars[right].type;
  if (left_t == right_t) {
    EmitArithExpr<calyx::Binop>({ left_t }, left, op, right);
    return;
  }

  switch (left_t) {
    case calyx::Var::Type::I32: {
      switch (right_t) {
        case calyx::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<i32, u32>>({ left_t }, right);
          EmitArithExpr<calyx::Binop>({ left_t }, left, op, current);
          return;
        case calyx::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<i64, i32>>({ right_t }, left);
          break;
        case calyx::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<u64, i32>>({ right_t }, left);
          break;
        case calyx::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, i32>>({ right_t }, left);
          break;
        case calyx::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, i32>>({ right_t }, left);
          break;
        default: throw std::runtime_error("Bad binop");
      }
      EmitArithExpr<calyx::Binop>({ right_t }, current, op, right);
      return;
    }
    case calyx::Var::Type::U32: {
      switch (right_t) {
        case calyx::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<i32, u32>>({ right_t }, left);
          break;
        case calyx::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<i64, u32>>({ right_t }, left);
          break;
        case calyx::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<u64, u32>>({ right_t }, left);
          break;
        case calyx::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, u32>>({ right_t }, left);
          break;
        case calyx::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, u32>>({ right_t }, left);
          break;
        default: throw std::runtime_error("Bad binop");
      }
      EmitArithExpr<calyx::Binop>({ right_t }, current, op, right);
      return;
    }
    case calyx::Var::Type::I64: {
      switch (right_t) {
        case calyx::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<i64, i32>>({ left_t }, right);
          EmitArithExpr<calyx::Binop>({ left_t }, left, op, current);
          return;
        case calyx::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<i64, u32>>({ left_t }, right);
          EmitArithExpr<calyx::Binop>({ left_t }, left, op, current);
          return;
        case calyx::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<i64, u64>>({ left_t }, right);
          EmitArithExpr<calyx::Binop>({ left_t }, left, op, current);
          return;
        case calyx::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, i64>>({ right_t }, left);
          EmitArithExpr<calyx::Binop>({ right_t }, current, op, right);
          return;
        case calyx::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, i64>>({ right_t }, left);
          EmitArithExpr<calyx::Binop>({ right_t }, current, op, right);
          return;
        default: throw std::runtime_error("Bad binop");
      }
    }
    case calyx::Var::Type::U64: {
      switch (right_t) {
        case calyx::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<u64, i32>>({left_t}, right);
          EmitArithExpr<calyx::Binop>({ left_t }, left, op, current);
          return;
        case calyx::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<u64, u32>>({left_t}, right);
          EmitArithExpr<calyx::Binop>({ left_t }, left, op, current);
          return;
        case calyx::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<i64, u64>>({right_t}, left);
          EmitArithExpr<calyx::Binop>({ right_t }, current, op, right);
          return;
        case calyx::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, u64>>({right_t}, left);
          EmitArithExpr<calyx::Binop>({ right_t }, current, op, right);
          return;
        case calyx::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, u64>>({right_t}, left);
          EmitArithExpr<calyx::Binop>({ right_t }, current, op, right);
          return;
        default: throw std::runtime_error("Bad binop");
      }
    }
    case calyx::Var::Type::Float: {
      switch (right_t) {
        case calyx::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, float>>({right_t}, left);
          EmitArithExpr<calyx::Binop>({ right_t }, current, op, right);
          return;
        case calyx::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<float, i32>>({left_t}, right);
          break;
        case calyx::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<float, u32>>({left_t}, right);
          break;
        case calyx::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<float, i64>>({left_t}, right);
          break;
        case calyx::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<float, u64>>({left_t}, right);
          break;
        default: throw std::runtime_error("Bad binop");
      }
      EmitArithExpr<calyx::Binop>({ left_t }, left, op, current);
      return;
    }
    case calyx::Var::Type::Double: {
      switch (right_t) {
        case calyx::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<double, i32>>({left_t}, right);
          break;
        case calyx::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<double, u32>>({left_t}, right);
          break;
        case calyx::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<double, i64>>({left_t}, right);
          break;
        case calyx::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<double, u64>>({left_t}, right);
          break;
        case calyx::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<double, float>>({left_t}, right);
          break;
        default: throw std::runtime_error("Bad binop");
      }
      EmitArithExpr<calyx::Binop>({ left_t }, left, op, current);
      return;
    }
    default: throw std::runtime_error("Bad binop");
  }
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
    emitter.Emit<calyx::AllocateCVar>(c_idx, size);

    if (decl.value.has_value()) {
      if (std::holds_alternative<pExpr>(decl.value.value())) {
        state.emplace(State::Read, 0);
        std::get<pExpr>(decl.value.value())->Visit(*this);
        state.pop();
        // current now holds the expression id that we want to assign with
        state.emplace(State::Assign, current);
        Identifier(decl.name).Visit(*this);
        state.pop();
      }
      else {
        // todo: handle initializer list
        throw std::runtime_error("Unimplemented");
      }
    }
  }
}

void ASTWalker::Visit(epi::taxy::FunctionDefinition& decl) {
  // same as normal compound statement besides arguments
  variables.NewLayer();
  // todo: arguments etc
  function = &decl;
  for (const auto& node : decl.body->stats) {
    node->Visit(*this);
  }
  for (auto var = variables.Top().rbegin(); var != variables.Top().rend(); var++) {
    emitter.Emit<calyx::DeallocateCVar>(var->second.idx, var->second.size);
  }
  variables.PopLayer();
}

void ASTWalker::Visit(Identifier& decl) {
  // after the AST, the only identifiers left are C variables
  auto type = c_types.Get(decl.name);
  auto cvar = variables.Get(decl.name);
  switch (state.top().first) {
    case State::Read: {
      if (type->IsArray()) {
        current = emitter.EmitExpr<calyx::LoadCVarAddr>({ calyx::Var::Type::Pointer, type->Deref()->Sizeof() }, cvar.idx);
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
  current = emitter.EmitExpr<calyx::Imm<i32>>({ detail::calyx_type_v<calyx::calyx_upcast_t<i8>> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<u8>& expr) {
  current = emitter.EmitExpr<calyx::Imm<u32>>({ detail::calyx_type_v<calyx::calyx_upcast_t<u8>> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<i16>& expr) {
  current = emitter.EmitExpr<calyx::Imm<i32>>({ detail::calyx_type_v<calyx::calyx_upcast_t<i16>> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<u16>& expr) {
  current = emitter.EmitExpr<calyx::Imm<u32>>({ detail::calyx_type_v<calyx::calyx_upcast_t<u16>> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<i32>& expr) {
  current = emitter.EmitExpr<calyx::Imm<i32>>({ detail::calyx_type_v<i32> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<u32>& expr) {
  current = emitter.EmitExpr<calyx::Imm<u32>>({ detail::calyx_type_v<u32> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<i64>& expr) {
  current = emitter.EmitExpr<calyx::Imm<i64>>({ detail::calyx_type_v<i64> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<u64>& expr) {
  current = emitter.EmitExpr<calyx::Imm<u64>>({ detail::calyx_type_v<u64> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<float>& expr) {
  current = emitter.EmitExpr<calyx::Imm<float>>({ detail::calyx_type_v<float> }, expr.value);
}

void ASTWalker::Visit(NumericalConstant<double>& expr) {
  current = emitter.EmitExpr<calyx::Imm<double>>({ detail::calyx_type_v<double> }, expr.value);
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
  switch (expr.op) {
    case TokenType::Minus: {
      cotyl::Assert(state.top().first == State::Read);

      // no need to push a new state
      expr.left->Visit(*this);
      EmitArithExpr<calyx::Unop>(emitter.vars[current].type, calyx::UnopType::Neg, current);
      return;
    }
    case TokenType::Plus: {
      cotyl::Assert(state.top().first == State::Read);
      // no need to push a new state
      // does nothing
      expr.left->Visit(*this);
      return;
    }
    case TokenType::Tilde: {
      cotyl::Assert(state.top().first == State::Read);
      // no need to push a new state
      expr.left->Visit(*this);
      EmitArithExpr<calyx::Unop>(emitter.vars[current].type, calyx::UnopType::BinNot, current);
      return;
    }
    default: {
      throw std::runtime_error("unimplemented unop");
    }
  }
}

void ASTWalker::Visit(Cast& expr) {
  cotyl::Assert(state.top().first == State::Read);
  // no need to push a new state
  expr.expr->Visit(*this);
  auto visitor = detail::EmitterTypeVisitor<detail::CastToEmitter>(*this, { current });
  expr.type->Visit(visitor);
}

void ASTWalker::Visit(Binop& expr) {
  cotyl::Assert(state.top().first == State::Read);
  // no need to push new state
  expr.left->Visit(*this);
  auto left = current;
  expr.right->Visit(*this);
  auto right = current;
  switch (expr.op) {
    case TokenType::Asterisk: BinopHelper(left, calyx::BinopType::Mul, right); return;
    case TokenType::Div: BinopHelper(left, calyx::BinopType::Div, right); return;
    case TokenType::Mod: BinopHelper(left, calyx::BinopType::Mod, right); return;
    case TokenType::Ampersand: BinopHelper(left, calyx::BinopType::BinAnd, right); return;
    case TokenType::BinOr: BinopHelper(left, calyx::BinopType::BinOr, right); return;
    case TokenType::BinXor: BinopHelper(left, calyx::BinopType::BinXor, right); return;
    case TokenType::Plus: {
      if (emitter.vars[left].type == calyx::Var::Type::Pointer) {
        throw std::runtime_error("Unimplemented: pointer add");
      }
      if (emitter.vars[right].type == calyx::Var::Type::Pointer) {
        throw std::runtime_error("Unimplemented: pointer add");
      }

      BinopHelper(left, calyx::BinopType::Add, right);
      return;
    }
    case TokenType::Minus: {
      if (emitter.vars[left].type == calyx::Var::Type::Pointer) {
        throw std::runtime_error("Unimplemented: pointer sub");
      }
      if (emitter.vars[right].type == calyx::Var::Type::Pointer) {
        throw std::runtime_error("Unimplemented: pointer sub");
      }

      BinopHelper(left, calyx::BinopType::Sub, right);
      return;
    }
    default:
      break;
  }
  // todo: shifts, comparisons, logical and/or
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Ternary& expr) {
  throw std::runtime_error("unimplemented");
}

void ASTWalker::Visit(Assignment& expr) {
  if (expr.op == TokenType::Assign) {
    state.emplace(State::Read, 0);
    expr.right->Visit(*this);
    state.pop();
    // current now holds the expression id that we want to assign with
    state.emplace(State::Assign, current);
    expr.left->Visit(*this);
    state.pop();
  }
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
    state.emplace(State::Read, 0);
    stat.expr->Visit(*this);
    auto visitor = detail::EmitterTypeVisitor<detail::ReturnEmitter>(*this, { current });
    function->signature->contained->Visit(visitor);
    state.pop();
  }
  else {
    emitter.Emit<calyx::Return<i32>>(0);
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
  for (auto var = variables.Top().rbegin(); var != variables.Top().rend(); var++) {
    emitter.Emit<calyx::DeallocateCVar>(var->second.idx, var->second.size);
  }
  variables.PopLayer();
}


}
