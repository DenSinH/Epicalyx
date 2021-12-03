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
    auto src_t = walker.emitter.vars[src].type;
    if ((src_t == calyx::Var::Type::Pointer) && (walker.emitter.vars[src].stride == stride)) {
      // "same pointer", no cast necessary
      return 0;
    }
    switch (src_t) {
      case calyx::Var::Type::I32: return walker.emitter.EmitExpr<calyx::Cast<T, i32>>({ calyx::Var::Type::Pointer, stride }, src);
      case calyx::Var::Type::U32: return walker.emitter.EmitExpr<calyx::Cast<T, u32>>({ calyx::Var::Type::Pointer, stride }, src);
      case calyx::Var::Type::I64: return walker.emitter.EmitExpr<calyx::Cast<T, i64>>({ calyx::Var::Type::Pointer, stride }, src);
      case calyx::Var::Type::U64: return walker.emitter.EmitExpr<calyx::Cast<T, u64>>({ calyx::Var::Type::Pointer, stride }, src);
      case calyx::Var::Type::Float: return walker.emitter.EmitExpr<calyx::Cast<T, float>>({ calyx::Var::Type::Pointer, stride }, src);
      case calyx::Var::Type::Double: return walker.emitter.EmitExpr<calyx::Cast<T, double>>({ calyx::Var::Type::Pointer, stride }, src);
      case calyx::Var::Type::Pointer: return walker.emitter.EmitExpr<calyx::Cast<T, calyx::Pointer>>({ calyx::Var::Type::Pointer, stride }, src);
      default:
        return 0;
    }
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
void ASTWalker::EmitIntegralExpr(calyx::Var::Type type, Args... args) {
  switch (type) {
    case calyx::Var::Type::I32: current = emitter.EmitExpr<Op<i32>>({ type }, args...); break;
    case calyx::Var::Type::U32: current = emitter.EmitExpr<Op<u32>>({ type }, args...); break;
    case calyx::Var::Type::I64: current = emitter.EmitExpr<Op<i64>>({ type }, args...); break;
    case calyx::Var::Type::U64: current = emitter.EmitExpr<Op<u64>>({ type }, args...); break;
    default:
      throw std::runtime_error("Bad type for integral expression");
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

template<template<typename T> class Op, typename... Args>
void ASTWalker::EmitCompare(calyx::Var::Type type, Args... args) {
  switch (type) {
    case calyx::Var::Type::I32: emitter.EmitExpr<Op<i32>>({ calyx::Var::Type::I32 }, args...); break;
    case calyx::Var::Type::U32: emitter.EmitExpr<Op<u32>>({ calyx::Var::Type::I32 }, args...); break;
    case calyx::Var::Type::I64: emitter.EmitExpr<Op<i64>>({ calyx::Var::Type::I32 }, args...); break;
    case calyx::Var::Type::U64: emitter.EmitExpr<Op<u64>>({ calyx::Var::Type::I32 }, args...); break;
    case calyx::Var::Type::Float: emitter.EmitExpr<Op<float>>({ calyx::Var::Type::I32 }, args...); break;
    case calyx::Var::Type::Double: emitter.EmitExpr<Op<double>>({ calyx::Var::Type::I32 }, args...); break;
    case calyx::Var::Type::Pointer: throw std::runtime_error("Unimplemented: pointer compare");
    default:
      throw std::runtime_error("Bad type for branch expression");
  }
}

template<template<typename T> class Op, typename... Args>
void ASTWalker::EmitBranch(calyx::Var::Type type, Args... args) {
  switch (type) {
    case calyx::Var::Type::I32: emitter.Emit<Op<i32>>(args...); break;
    case calyx::Var::Type::U32: emitter.Emit<Op<u32>>(args...); break;
    case calyx::Var::Type::I64: emitter.Emit<Op<i64>>(args...); break;
    case calyx::Var::Type::U64: emitter.Emit<Op<u64>>(args...); break;
    case calyx::Var::Type::Float: emitter.Emit<Op<float>>(args...); break;
    case calyx::Var::Type::Double: emitter.Emit<Op<double>>(args...); break;
    case calyx::Var::Type::Pointer: emitter.Emit<Op<calyx::Pointer>>(args...); break;
    default:
      throw std::runtime_error("Bad type for branch expression");
  }
}

void ASTWalker::BinopHelper(calyx::var_index_t left, calyx::BinopType op, calyx::var_index_t right) {
  auto casted = BinopCastHelper(left, right);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wbraced-scalar-init"
  EmitArithExpr<calyx::Binop>({casted.type}, casted.left, op, casted.right);
#pragma clang diagnostic pop
}

ASTWalker::BinopCastResult ASTWalker::BinopCastHelper(calyx::var_index_t left, calyx::var_index_t right) {
  auto left_t = emitter.vars[left].type;
  auto right_t = emitter.vars[right].type;
  if (left_t == right_t) {
    return {left_t, left, right};
  }

  switch (left_t) {
    case calyx::Var::Type::I32: {
      switch (right_t) {
        case calyx::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<u32, i32>>({ right_t }, left);
          return {right_t, current, right};
        case calyx::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<i64, i32>>({ right_t }, left);
          return {right_t, current, right};
        case calyx::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<u64, i32>>({ right_t }, left);
          return {right_t, current, right};
        case calyx::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, i32>>({ right_t }, left);
          return {right_t, current, right};
        case calyx::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, i32>>({ right_t }, left);
          return {right_t, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case calyx::Var::Type::U32: {
      switch (right_t) {
        case calyx::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<u32, i32>>({ left_t }, right);
          return {left_t, left, current};
        case calyx::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<i64, u32>>({ right_t }, left);
          return {right_t, current, right};
        case calyx::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<u64, u32>>({ right_t }, left);
          return {right_t, current, right};
        case calyx::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, u32>>({ right_t }, left);
          return {right_t, current, right};
        case calyx::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, u32>>({ right_t }, left);
          return {right_t, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case calyx::Var::Type::I64: {
      switch (right_t) {
        case calyx::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<i64, i32>>({ left_t }, right);
          return {left_t, left, current};
        case calyx::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<i64, u32>>({ left_t }, right);
          return {left_t, left, current};
        case calyx::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<u64, i64>>({ right_t }, left);
          return {right_t, current, right};
        case calyx::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, i64>>({ right_t }, left);
          return {right_t, current, right};
        case calyx::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, i64>>({ right_t }, left);
          return {right_t, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case calyx::Var::Type::U64: {
      switch (right_t) {
        case calyx::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<u64, i32>>({left_t}, right);
          return {left_t, left, current};
        case calyx::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<u64, u32>>({left_t}, right);
          return {left_t, left, current};
        case calyx::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<u64, i64>>({left_t}, right);
          return {left_t, left, current};
        case calyx::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, u64>>({right_t}, left);
          return {right_t, current, right};
        case calyx::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, u64>>({right_t}, left);
          return {right_t, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case calyx::Var::Type::Float: {
      switch (right_t) {
        case calyx::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, float>>({right_t}, left);
          return {right_t, current, right};
        case calyx::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<float, i32>>({left_t}, right);
          return {left_t, left, current};
        case calyx::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<float, u32>>({left_t}, right);
          return {left_t, left, current};
        case calyx::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<float, i64>>({left_t}, right);
          return {left_t, left, current};
        case calyx::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<float, u64>>({left_t}, right);
          return {left_t, left, current};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case calyx::Var::Type::Double: {
      switch (right_t) {
        case calyx::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<double, i32>>({left_t}, right);
          return {left_t, left, current};
        case calyx::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<double, u32>>({left_t}, right);
          return {left_t, left, current};
        case calyx::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<double, i64>>({left_t}, right);
          return {left_t, left, current};
        case calyx::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<double, u64>>({left_t}, right);
          return {left_t, left, current};
        case calyx::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<double, float>>({left_t}, right);
          return {left_t, left, current};
        default: throw std::runtime_error("Bad binop");
      }
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
        state.push({State::Read, {}});
        std::get<pExpr>(decl.value.value())->Visit(*this);
        state.pop();
        // current now holds the expression id that we want to assign with
        state.push({State::Assign, {.var = current}});
        Identifier(decl.name).Visit(*this);
        state.pop();
      }
      else {
        // todo: handle initializer list
        throw std::runtime_error("Unimplemented: initializer list declaration");
      }
    }
  }
}

void ASTWalker::Visit(epi::taxy::FunctionDefinition& decl) {
  // same as normal compound statement besides arguments
  variables.NewLayer();
  // todo: arguments etc
  function = &decl;
  reachable = true;
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
  if (state.empty()) {
    // statement has no effect
    return;
  }
  if (!reachable) {
    return;
  }

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
              *this, { cvar.idx, state.top().second.var }
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
    case State::ConditionalBranch: {
      // first part is same as read
      state.push({State::Read, {}});
      Visit(decl);
      state.pop();

      // emit branch
      // branch to false block if 0, otherwise go to true block
      auto block = state.top().second.false_block;
      EmitBranch<calyx::BranchCompareImm>(emitter.vars[current].type, block, current, calyx::CmpType::Eq, 0);
      emitter.Emit<calyx::UnconditionalBranch>(state.top().second.true_block);
      break;
    }
    default: {
      throw std::runtime_error("Bad declaration state");
    }
  }
}

template<typename T>
void ASTWalker::ConstVisitImpl(NumericalConstant<T>& expr) {
  cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  if (!reachable) return;
  if (state.top().first == State::ConditionalBranch) {
    if (expr.value) {
      emitter.Emit<calyx::UnconditionalBranch>(state.top().second.true_block);
    }
    else {
      emitter.Emit<calyx::UnconditionalBranch>(state.top().second.false_block);
    }
  }
  else {
    current = emitter.EmitExpr<calyx::Imm<calyx::calyx_upcast_t<T>>>({ detail::calyx_type_v<i32> }, expr.value);
  }
}

template void ASTWalker::ConstVisitImpl(NumericalConstant<i8>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<u8>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<i16>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<u16>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<i32>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<u32>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<i64>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<u64>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<float>&);
template void ASTWalker::ConstVisitImpl(NumericalConstant<double>&);

void ASTWalker::Visit(StringConstant& expr) {
  // load from rodata
  throw std::runtime_error("unimplemented: string constant");
}

void ASTWalker::Visit(ArrayAccess& expr) {
  throw std::runtime_error("unimplemented: array access");
}

void ASTWalker::Visit(FunctionCall& expr) {
  throw std::runtime_error("unimplemented: function call");
}

void ASTWalker::Visit(MemberAccess& expr) {
  throw std::runtime_error("unimplemented: member access");
}

void ASTWalker::Visit(TypeInitializer& expr) {
  throw std::runtime_error("unimplemented: type initializer");
}

void ASTWalker::Visit(PostFix& expr) {
  cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  if (!reachable) return;

  bool conditional_branch = !state.empty() && state.top().first == State::ConditionalBranch;

  switch (expr.op) {
    case TokenType::Incr: {
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();
      auto read = current;
      auto type = emitter.vars[current].type;
      if (type == calyx::Var::Type::Pointer) {
        // todo
      }
      else if (type == calyx::Var::Type::Struct) {
        throw std::runtime_error("Bad expression for post-increment: struct");
      }
      else {
        EmitArithExpr<calyx::BinopImm>(type, read, calyx::BinopType::Add, 1);
      }

      // write back
      state.push({State::Assign, {.var = current}});
      expr.left->Visit(*this);
      state.pop();
      // restore read value for next expression
      current = read;

      // need to check for conditional branches
      break;
    }
    case TokenType::Decr: {
      cotyl::Assert(state.empty() || state.top().first == State::Read);
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();
      auto read = current;
      auto type = emitter.vars[current].type;
      if (type == calyx::Var::Type::Pointer) {
        // todo
      }
      else if (type == calyx::Var::Type::Struct) {
        throw std::runtime_error("Bad expression for post-decrement: struct");
      }
      else {
        EmitArithExpr<calyx::BinopImm>(type, read, calyx::BinopType::Sub, 1);
      }

      // write back
      state.push({State::Assign, {.var = current}});
      expr.left->Visit(*this);
      state.pop();
      // restore read value for next expression
      current = read;

      // need to check for conditional branches
      break;
    }
    default: {
      throw std::runtime_error("Unreachable");
    }
  }

  if (conditional_branch) {
    auto block = state.top().second.false_block;
    EmitBranch<calyx::BranchCompareImm>(emitter.vars[current].type, block, current, calyx::CmpType::Eq, 0);
  }
}

void ASTWalker::Visit(Unary& expr) {
  cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  if (!reachable) return;

  const bool conditional_branch = !state.empty() && state.top().first == State::ConditionalBranch;

  switch (expr.op) {
    case TokenType::Minus: {
      // no need to push a new state
      expr.left->Visit(*this);
      if (!conditional_branch) {
        EmitArithExpr<calyx::Unop>(emitter.vars[current].type, calyx::UnopType::Neg, current);
      }
      // return, no need to check for conditional branches, is handled in visiting the expr
      return;
    }
    case TokenType::Plus: {
      // no need to push a new state
      // does nothing
      // return, no need to check for conditional branches, is handled in visiting the expr
      expr.left->Visit(*this);
      return;
    }
    case TokenType::Tilde: {
      // no need to push a new state
      expr.left->Visit(*this);
      EmitArithExpr<calyx::Unop>(emitter.vars[current].type, calyx::UnopType::BinNot, current);
      // break, need to check for conditional branches
      break;
    }
    case TokenType::Exclamation: {
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();

      if (conditional_branch) {
        auto block = state.top().second.false_block;
        // branch to false if current != 0 (if current == 0, then !current is true)
        EmitBranch<calyx::BranchCompareImm>(emitter.vars[current].type, block, current, calyx::CmpType::Ne, 0);
      }
      else {
        EmitCompare<calyx::CompareImm>(emitter.vars[current].type, current, calyx::CmpType::Eq, 0);
      }

      // return, no need to check for conditional branches, already handled
      return;
    }
    case TokenType::Incr: {
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();

      auto type = emitter.vars[current].type;
      calyx::var_index_t stored;
      if (type == calyx::Var::Type::Pointer) {
        // todo
      }
      else if (type == calyx::Var::Type::Struct) {
        throw std::runtime_error("Bad expression for pre-increment: struct");
      }
      else {
        EmitArithExpr<calyx::BinopImm>(type, current, calyx::BinopType::Add, 1);
      }
      stored = current;

      // write back
      state.push({State::Assign, {.var = current}});
      expr.left->Visit(*this);
      state.pop();

      // restore stored value
      current = stored;
      // break, need to check for conditional branches
      break;
    }
    case TokenType::Decr: {
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();
      auto type = emitter.vars[current].type;
      calyx::var_index_t stored;
      if (type == calyx::Var::Type::Pointer) {
        // todo
      }
      else if (type == calyx::Var::Type::Struct) {
        throw std::runtime_error("Bad expression for pre-decrement: struct");
      }
      else {
        EmitArithExpr<calyx::BinopImm>(type, current, calyx::BinopType::Sub, 1);
      }
      stored = current;

      // write back
      state.push({State::Assign, {.var = current}});
      expr.left->Visit(*this);
      state.pop();

      // restore stored value
      current = stored;

      // break, need to check for conditional branches
      break;
    }
    default: {
      throw std::runtime_error("unimplemented unop");
    }
  }

  if (conditional_branch) {
    auto block = state.top().second.false_block;
    EmitBranch<calyx::BranchCompareImm>(emitter.vars[current].type, block, current, calyx::CmpType::Eq, 0);
  }
}

void ASTWalker::Visit(Cast& expr) {
  cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  // no need to push a new state
  expr.expr->Visit(*this);

  // only need to emit an actual cast if we are not checking for 0
  if (state.empty() || state.top().first != State::ConditionalBranch) {
    auto visitor = detail::EmitterTypeVisitor<detail::CastToEmitter>(*this, { current });
    expr.type->Visit(visitor);
  }
}

void ASTWalker::Visit(Binop& expr) {
  cotyl::Assert(state.empty() || state.top().first == State::Read || state.top().first == State::ConditionalBranch);
  if (!reachable) return;

  // only need to push a new state for conditional branches
  const bool conditional_branch = !state.empty() && (state.top().first == State::ConditionalBranch);
  if (conditional_branch) {
    state.push({State::Read, {}});
  }
  expr.left->Visit(*this);
  auto left = current;
  expr.right->Visit(*this);
  auto right = current;
  if (conditional_branch) {
    state.pop();
  }

  switch (expr.op) {
    case TokenType::Asterisk: BinopHelper(left, calyx::BinopType::Mul, right); break;
    case TokenType::Div: BinopHelper(left, calyx::BinopType::Div, right); break;
    case TokenType::Mod: BinopHelper(left, calyx::BinopType::Mod, right); break;
    case TokenType::Ampersand: BinopHelper(left, calyx::BinopType::BinAnd, right); break;
    case TokenType::BinOr: BinopHelper(left, calyx::BinopType::BinOr, right); break;
    case TokenType::BinXor: BinopHelper(left, calyx::BinopType::BinXor, right); break;
    case TokenType::Plus: {
      if (emitter.vars[left].type == calyx::Var::Type::Pointer) {
        throw std::runtime_error("Unimplemented: pointer add");
      }
      if (emitter.vars[right].type == calyx::Var::Type::Pointer) {
        throw std::runtime_error("Unimplemented: pointer add");
      }

      BinopHelper(left, calyx::BinopType::Add, right);
      break;
    }
    case TokenType::Minus: {
      if (emitter.vars[left].type == calyx::Var::Type::Pointer) {
        throw std::runtime_error("Unimplemented: pointer sub");
      }
      if (emitter.vars[right].type == calyx::Var::Type::Pointer) {
        throw std::runtime_error("Unimplemented: pointer sub");
      }

      BinopHelper(left, calyx::BinopType::Sub, right);
      break;
    }
    case TokenType::Less:
    case TokenType::LessEqual:
    case TokenType::GreaterEqual:
    case TokenType::Greater:
    case TokenType::Equal:
    case TokenType::NotEqual: {
      auto casted = BinopCastHelper(left, right);
      calyx::CmpType cmp_type;
      calyx::CmpType inverse;
      switch (expr.op) {
        case TokenType::Less:
          cmp_type = calyx::CmpType::Lt;
          inverse = calyx::CmpType::Ge;
          break;
        case TokenType::LessEqual:
          cmp_type = calyx::CmpType::Le;
          inverse = calyx::CmpType::Gt;
          break;
        case TokenType::GreaterEqual:
          cmp_type = calyx::CmpType::Ge;
          inverse = calyx::CmpType::Lt;
          break;
        case TokenType::Greater:
          cmp_type = calyx::CmpType::Gt;
          inverse = calyx::CmpType::Le;
          break;
        case TokenType::Equal:
          cmp_type = calyx::CmpType::Eq;
          inverse = calyx::CmpType::Ne;
          break;
        case TokenType::NotEqual:
          cmp_type = calyx::CmpType::Ne;
          inverse = calyx::CmpType::Eq;
          break;
        default:
          throw std::runtime_error("unreachable");
      }

      if (conditional_branch) {
        auto block = state.top().second.false_block;
        EmitBranch<calyx::BranchCompare>(casted.type, block, casted.left, inverse, casted.right);
      }
      else {
        EmitCompare<calyx::Compare>(casted.type, casted.left, cmp_type, casted.right);
      }
      // no need to check for conditional branching after this
      return;
    }
    case TokenType::LShift:
    case TokenType::RShift: {
      cotyl::Assert(!cotyl::Is(emitter.vars[left].type).AnyOf<calyx::Var::Type::Float, calyx::Var::Type::Double>());
      cotyl::Assert(!cotyl::Is(emitter.vars[right].type).AnyOf<calyx::Var::Type::Float, calyx::Var::Type::Double>());
      switch (emitter.vars[right].type) {
        case calyx::Var::Type::I32: {
          right = emitter.EmitExpr<calyx::Cast<u32, i32>>({ calyx::Var::Type::U32 }, right);
          break;
        }
        case calyx::Var::Type::I64: {
          right = emitter.EmitExpr<calyx::Cast<u32, i64>>({ calyx::Var::Type::U32 }, right);
          break;
        }
        case calyx::Var::Type::U64: {
          right = emitter.EmitExpr<calyx::Cast<u32, u64>>({ calyx::Var::Type::U32 }, right);
          break;
        }
        case calyx::Var::Type::U32: break;
        default: {
          throw std::runtime_error("Bad operand type for shift amount");
        }
      }
      if (expr.op == TokenType::LShift) {
        EmitIntegralExpr<calyx::Shift>(emitter.vars[left].type, left, calyx::ShiftType::Left, right);
      }
      else {
        EmitIntegralExpr<calyx::Shift>(emitter.vars[left].type, left, calyx::ShiftType::Right, right);
      }
      break;
    }
    default:
      throw std::runtime_error("Bad binop");
  }

  if (conditional_branch) {
    auto type = emitter.vars[current].type;
    auto block = state.top().second.false_block;
    EmitBranch<calyx::BranchCompareImm>(type, block, current, calyx::CmpType::Eq, 0);
  }
}

void ASTWalker::Visit(Ternary& expr) {
  throw std::runtime_error("unimplemented: ternary");
}

void ASTWalker::Visit(Assignment& expr) {
  cotyl::Assert(state.empty() || state.top().first == State::Read);
  if (!reachable) return;

  state.push({State::Read, {}});
  expr.right->Visit(*this);
  state.pop();

  // current now holds the expression id that we want to assign with
  auto right = current;
  calyx::BinopType op;
  switch (expr.op) {
    case TokenType::IMul: op = calyx::BinopType::Mul; break;
    case TokenType::IDiv: op = calyx::BinopType::Div; break;
    case TokenType::IMod: op = calyx::BinopType::Mod; break;
    case TokenType::IPlus: op = calyx::BinopType::Add; break;
    case TokenType::IMinus: op = calyx::BinopType::Sub; break;
    case TokenType::ILShift:
    case TokenType::IRShift: {
      // same as binop LShift / RShift
      state.push({State::Read, {}});
      expr.left->Visit(*this);
      state.pop();

      auto left = current;
      cotyl::Assert(!cotyl::Is(emitter.vars[left].type).AnyOf<calyx::Var::Type::Float, calyx::Var::Type::Double>());
      cotyl::Assert(!cotyl::Is(emitter.vars[right].type).AnyOf<calyx::Var::Type::Float, calyx::Var::Type::Double>());
      switch (emitter.vars[right].type) {
        case calyx::Var::Type::I32: {
          right = emitter.EmitExpr<calyx::Cast<u32, i32>>({ calyx::Var::Type::U32 }, right);
          break;
        }
        case calyx::Var::Type::I64: {
          right = emitter.EmitExpr<calyx::Cast<u32, i64>>({ calyx::Var::Type::U32 }, right);
          break;
        }
        case calyx::Var::Type::U64: {
          right = emitter.EmitExpr<calyx::Cast<u32, u64>>({ calyx::Var::Type::U32 }, right);
          break;
        }
        case calyx::Var::Type::U32: break;
        default: {
          throw std::runtime_error("Bad operand type for shift amount");
        }
      }
      if (expr.op == TokenType::ILShift) {
        EmitIntegralExpr<calyx::Shift>(emitter.vars[left].type, left, calyx::ShiftType::Left, right);
      }
      else {
        EmitIntegralExpr<calyx::Shift>(emitter.vars[left].type, left, calyx::ShiftType::Right, right);
      }

      // do assignment
      state.push({State::Assign, {.var = current}});
      expr.left->Visit(*this);
      state.pop();

      // todo: conditional branch
      return;
    }
    case TokenType::IAnd: op = calyx::BinopType::BinAnd; break;
    case TokenType::IOr: op = calyx::BinopType::BinOr; break;
    case TokenType::IXor: op = calyx::BinopType::BinXor; break;
    case TokenType::Assign: {
      state.push({State::Assign, {.var = right}});
      expr.left->Visit(*this);
      state.pop();

      // todo: conditional branch
      return;
    }
    default: {
      throw std::runtime_error("Invalid assignment statement");
    }
  }

  // visit left for binop assignment
  state.push({State::Read, {}});
  expr.left->Visit(*this);
  state.pop();

  auto left = current;

  // emit binop
  BinopHelper(left, op, right);

  auto result = current;

  // do assignment
  state.push({State::Assign, {.var = current}});
  expr.left->Visit(*this);
  state.pop();

  // set current to the result variable
  current = result;
}

void ASTWalker::Visit(Empty& stat) {

}

void ASTWalker::Visit(If& stat) {
  if (!reachable) return;

  auto true_block = emitter.MakeBlock();
  auto false_block = emitter.MakeBlock();
  calyx::var_index_t post_block;
  if (stat._else) {
    post_block = emitter.MakeBlock();
  }
  else {
    post_block = false_block;
  }

  state.push({State::ConditionalBranch, {.true_block = true_block, .false_block = false_block}});
  stat.cond->Visit(*this);
  state.pop();

  // true, then jump to post block
  // emitter should have emitted a branch to the false block if needed
  emitter.Emit<calyx::UnconditionalBranch>(true_block);
  emitter.SelectBlock(true_block);
  stat.stat->Visit(*this);
  bool true_reachable = reachable;
  if (reachable) {
    emitter.Emit<calyx::UnconditionalBranch>(post_block);
  }
  reachable = true;

  bool false_reachable = true;
  if (stat._else) {
    emitter.SelectBlock(false_block);
    stat._else->Visit(*this);
    false_reachable = reachable;
    if (reachable) {
      // unreachable means we already returned or branched somewhere else
      emitter.Emit<calyx::UnconditionalBranch>(post_block);
    }
  }

  emitter.SelectBlock(post_block);
  reachable = true_reachable || false_reachable;
  cotyl::Assert(emitter.program[post_block].empty());
}

void ASTWalker::Visit(While& stat) {
  if (!reachable) return;

  auto cond_block = emitter.MakeBlock();
  emitter.Emit<calyx::UnconditionalBranch>(cond_block);
  emitter.SelectBlock(cond_block);

  auto loop_block = emitter.MakeBlock();
  auto post_block = emitter.MakeBlock();

  state.push({State::ConditionalBranch, {.true_block = loop_block, .false_block = post_block}});
  stat.cond->Visit(*this);
  state.pop();

  // branch to false block (post_block) should have been emitted by the cond Visit
  emitter.Emit<calyx::UnconditionalBranch>(loop_block);
  emitter.SelectBlock(loop_block);

  break_stack.push(post_block);
  continue_stack.push(cond_block);

  stat.stat->Visit(*this);

  break_stack.pop();
  continue_stack.pop();

  // loop back to the condition block
  emitter.Emit<calyx::UnconditionalBranch>(cond_block);

  // go to block after loop
  emitter.SelectBlock(post_block);
}

void ASTWalker::Visit(DoWhile& stat) {
  if (!reachable) return;

  auto loop_block = emitter.MakeBlock();
  auto cond_block = emitter.MakeBlock();
  auto post_block = emitter.MakeBlock();

  // branch to loop block always happens at least once
  emitter.Emit<calyx::UnconditionalBranch>(loop_block);
  emitter.SelectBlock(loop_block);

  break_stack.push(post_block);
  continue_stack.push(cond_block);

  stat.stat->Visit(*this);

  break_stack.pop();
  continue_stack.pop();

  // go to cond block
  // this is a separate block for continue statements
  emitter.Emit<calyx::UnconditionalBranch>(cond_block);
  emitter.SelectBlock(cond_block);

  state.push({State::ConditionalBranch, {.true_block = loop_block, .false_block = post_block}});
  stat.cond->Visit(*this);
  state.pop();

  // loop back to the loop block if condition was met (false case should be handled by cond visit)
  emitter.Emit<calyx::UnconditionalBranch>(loop_block);

  // go to block after loop
  emitter.SelectBlock(post_block);
}

void ASTWalker::Visit(For& stat) {
  if (!reachable) return;

  auto init_block = emitter.MakeBlock();
  auto cond_block = emitter.MakeBlock();
  auto update_block = emitter.MakeBlock();
  auto loop_block = emitter.MakeBlock();
  auto post_block = emitter.MakeBlock();

  // new scope for declarations in for loop
  variables.NewLayer();

  // always go to initialization
  emitter.Emit<calyx::UnconditionalBranch>(init_block);
  emitter.SelectBlock(init_block);
  for (auto& decl : stat.decls) {
    decl->Visit(*this);
  }
  for (auto& init : stat.inits) {
    init->Visit(*this);
  }

  // go to condition
  emitter.Emit<calyx::UnconditionalBranch>(cond_block);
  emitter.SelectBlock(cond_block);

  state.push({State::ConditionalBranch, {.true_block = loop_block, .false_block = post_block}});
  stat.cond->Visit(*this);
  state.pop();

  // false_block case should have been handled by condition
  emitter.Emit<calyx::UnconditionalBranch>(loop_block);
  emitter.SelectBlock(loop_block);

  break_stack.push(post_block);
  continue_stack.push(update_block);

  // loop body
  stat.stat->Visit(*this);

  break_stack.pop();
  continue_stack.pop();

  // go to update and loop back to condition
  emitter.Emit<calyx::UnconditionalBranch>(update_block);
  emitter.SelectBlock(update_block);
  for (auto& update : stat.updates) {
    update->Visit(*this);
  }
  emitter.Emit<calyx::UnconditionalBranch>(cond_block);

  // pop variables layer
  // todo: dealloc?
//  for (auto var = variables.Top().rbegin(); var != variables.Top().rend(); var++) {
//    emitter.Emit<calyx::DeallocateCVar>(var->second.idx, var->second.size);
//  }
  variables.PopLayer();

  // go to block after loop
  emitter.SelectBlock(post_block);
}

void ASTWalker::Visit(Label& stat) {
  if (!emitter.labels.contains(stat.name)) {
    auto block = emitter.MakeBlock();
    emitter.labels.emplace(stat.name, block);
    emitter.Emit<calyx::UnconditionalBranch>(block);
    emitter.SelectBlock(block);
  }
  else {
    auto block = emitter.labels.at(stat.name);
    emitter.Emit<calyx::UnconditionalBranch>(block);
    emitter.SelectBlock(block);
  }
  stat.stat->Visit(*this);
}

void ASTWalker::Visit(Switch& stat) {
  state.push({State::Read, {}});
  stat.expr->Visit(*this);
  state.pop();

  auto right = current;
  switch (emitter.vars[right].type) {
    case calyx::Var::Type::I32: {
      right = emitter.EmitExpr<calyx::Cast<i64, i32>>({ calyx::Var::Type::U32 }, right);
      break;
    }
    case calyx::Var::Type::U32: {
      right = emitter.EmitExpr<calyx::Cast<i64, u32>>({ calyx::Var::Type::U32 }, right);
      break;
    }
    case calyx::Var::Type::U64: {
      right = emitter.EmitExpr<calyx::Cast<i64, u64>>({ calyx::Var::Type::U32 }, right);
      break;
    }
    case calyx::Var::Type::I64: break;
    default: {
      throw std::runtime_error("Bad operand type for switch statement");
    }
  }

  auto select = emitter.Emit<calyx::Select>(right);
  auto post_block = emitter.MakeBlock();

  break_stack.push(post_block);
  select_stack.push(select);
  reachable = false;  // to prevent branch after select
  stat.stat->Visit(*this);
  select_stack.pop();
  break_stack.pop();

  emitter.SelectBlock(post_block);

  // todo: return from all cases
  reachable = true;
}

void ASTWalker::Visit(Case& stat) {
  cotyl::Assert(!select_stack.empty(), "Invalid case statement");
  auto* select = select_stack.top();
  cotyl::Assert(!select->table.contains(stat.expr), "Duplicate case statement");

  auto block = emitter.MakeBlock();
  if (reachable) {
    // check for fallthrough and don't emit branch if there is no fallthrough
    emitter.Emit<calyx::UnconditionalBranch>(block);
  }
  reachable = true;

  select->table.emplace(stat.expr, block);

  emitter.SelectBlock(block);
  stat.stat->Visit(*this);
}

void ASTWalker::Visit(Default& stat) {
  cotyl::Assert(!select_stack.empty(), "Invalid default statement");
  auto* select = select_stack.top();
  cotyl::Assert(select->_default == 0, "Duplicate default statement");

  auto block = emitter.MakeBlock();
  if (reachable) {
    // check for fallthrough and don't emit branch if there is no fallthrough
    emitter.Emit<calyx::UnconditionalBranch>(block);
  }
  reachable = true;

  select->_default = block;

  emitter.SelectBlock(block);
  stat.stat->Visit(*this);
}

void ASTWalker::Visit(Goto& stat) {
  if (!emitter.labels.contains(stat.label)) {
    auto block = emitter.MakeBlock();
    emitter.labels.emplace(stat.label, block);
    emitter.Emit<calyx::UnconditionalBranch>(block);
  }
  else {
    emitter.Emit<calyx::UnconditionalBranch>(emitter.labels.at(stat.label));
  }
}

void ASTWalker::Visit(Return& stat) {
  if (!reachable) return;

  if (stat.expr) {
    state.push({State::Read, {}});
    stat.expr->Visit(*this);
    auto visitor = detail::EmitterTypeVisitor<detail::ReturnEmitter>(*this, { current });
    function->signature->contained->Visit(visitor);
    state.pop();
  }
  else {
    emitter.Emit<calyx::Return<i32>>(0);
  }
  reachable = false;
}

void ASTWalker::Visit(Break& stat) {
  cotyl::Assert(!break_stack.empty());
  reachable = false;
  emitter.Emit<calyx::UnconditionalBranch>(break_stack.top());
}

void ASTWalker::Visit(Continue& stat) {
  cotyl::Assert(!continue_stack.empty());
  reachable = false;
  emitter.Emit<calyx::UnconditionalBranch>(continue_stack.top());
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
