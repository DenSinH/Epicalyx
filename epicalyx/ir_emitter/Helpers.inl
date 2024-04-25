#include "Default.h"
#include "Decltype.h"

namespace epi::detail {

using namespace ast;

template<typename T> struct calyx_var_type;
template<> struct calyx_var_type<i32> { static constexpr auto value = Emitter::Var::Type::I32; };
template<> struct calyx_var_type<u32> { static constexpr auto value = Emitter::Var::Type::U32; };
template<> struct calyx_var_type<i64> { static constexpr auto value = Emitter::Var::Type::I64; };
template<> struct calyx_var_type<u64> { static constexpr auto value = Emitter::Var::Type::U64; };
template<> struct calyx_var_type<float> { static constexpr auto value = Emitter::Var::Type::Float; };
template<> struct calyx_var_type<double> { static constexpr auto value = Emitter::Var::Type::Double; };
template<> struct calyx_var_type<calyx::Pointer> { static constexpr auto value = Emitter::Var::Type::Pointer; };
template<> struct calyx_var_type<calyx::Struct> { static constexpr auto value = Emitter::Var::Type::Struct; };
template<typename T>
constexpr auto calyx_var_type_v = calyx_var_type<T>::value;

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


/*
 * Generic type visitor to be used for different type-dependent operations.
 * One can write their own emitter that handles value types / pointer types / struct types differently.
 * This removes the boilerplate of the rest of the visitor pattern code.
 * */
template<template<typename T> class emit>
struct EmitterTypeVisitor {
  using args_t = func_args_t<decltype(emit<i32>::emit_value)>;
  using return_t = func_return_t<decltype(emit<i32>::emit_value)>;

  EmitterTypeVisitor(ASTWalker& walker, args_t&& args) :
          walker(walker), args{std::move(args)} {

  }

  void Visit(const type::AnyType& type) {
    type.visit<void>(
      [](const type::VoidType&) {
        throw std::runtime_error("Incomplete type");
      },
      [&](const type::PointerType& ptr) {
        VisitPointerImpl(ptr.Stride());
      },
      [&](const type::FunctionType& ptr) {
        VisitPointerImpl(0);
      },
      [](const type::StructType&) {
        throw cotyl::UnimplementedException();
      },
      [](const type::UnionType&) {
        throw cotyl::UnimplementedException();
      },
      [&]<typename T>(const type::ValueType<T>& value) {
        VisitValueImpl<T>();
      },
      // exhaustive variant access
      [](const auto& invalid) { static_assert(!sizeof(invalid)); }
    );
  }

private:
  ASTWalker& walker;
  args_t args;

  template<typename T>
  void VisitValueImpl() {
    if constexpr(std::is_same_v<return_t, var_index_t>) {
      walker.current = std::apply([&](auto&&... _args) { return emit<T>::emit_value(walker, std::move(_args)...); }, std::move(args));
    }
    else if constexpr(std::is_same_v<return_t, void>) {
      std::apply([&](auto&&... _args) { return emit<T>::emit_value(walker, std::move(_args)...); }, std::move(args));
    }
    else {
      []<bool flag = false> { static_assert(flag); }();
    }
  }

  void VisitPointerImpl(u64 stride) {
    if constexpr(std::is_same_v<return_t, var_index_t>) {
      walker.current = std::apply([&](auto&&... _args){ return emit<calyx::Pointer>::emit_pointer(walker, stride, std::move(_args)...); }, std::move(args));
    }
    else if constexpr(std::is_same_v<return_t , void>) {
      std::apply([&](auto&&... _args){ return emit<calyx::Pointer>::emit_pointer(walker, stride, std::move(_args)...); }, std::move(args));
    }
    else {
      []<bool flag = false> { static_assert(flag); }();
    }
  }
};

template<typename T>
struct CastToEmitter {
  static var_index_t do_cast(ASTWalker& walker, Emitter::Var::Type src_t, var_index_t src_idx) {
    if (!std::is_same_v<calyx::calyx_upcast_t<T>, T> || (calyx_var_type_v<calyx::calyx_upcast_t<T>> != src_t)) {
      using upcast = typename calyx::calyx_upcast_t<T>;
      switch (src_t) {
        case Emitter::Var::Type::I32: return walker.emitter.EmitExpr<calyx::Cast<T, i32>>({ calyx_var_type_v<upcast> }, src_idx);
        case Emitter::Var::Type::U32: return walker.emitter.EmitExpr<calyx::Cast<T, u32>>({ calyx_var_type_v<upcast> }, src_idx);
        case Emitter::Var::Type::I64: return walker.emitter.EmitExpr<calyx::Cast<T, i64>>({ calyx_var_type_v<upcast> }, src_idx);
        case Emitter::Var::Type::U64: return walker.emitter.EmitExpr<calyx::Cast<T, u64>>({ calyx_var_type_v<upcast> }, src_idx);
        case Emitter::Var::Type::Float: return walker.emitter.EmitExpr<calyx::Cast<T, float>>({ calyx_var_type_v<upcast> }, src_idx);
        case Emitter::Var::Type::Double: return walker.emitter.EmitExpr<calyx::Cast<T, double>>({ calyx_var_type_v<upcast> }, src_idx);
        case Emitter::Var::Type::Pointer: return walker.emitter.EmitExpr<calyx::Cast<T, calyx::Pointer>>({ calyx_var_type_v<upcast> }, src_idx);
        default:
          return 0;
      }
    }
    return 0;
  }

  static var_index_t emit_value(ASTWalker& walker, var_index_t src) {
    var_index_t cast = do_cast(walker, walker.emitter.vars[src].type, src);
    return cast ? cast : src;
  }

  static var_index_t emit_pointer(ASTWalker& walker, u64 stride, var_index_t src) {
    auto src_t = walker.emitter.vars[src].type;
    if ((src_t == Emitter::Var::Type::Pointer) && (walker.emitter.vars[src].stride == stride)) {
      // "same pointer", no cast necessary
      return src;
    }
    switch (src_t) {
      case Emitter::Var::Type::I32: return walker.emitter.EmitExpr<calyx::Cast<T, i32>>({ Emitter::Var::Type::Pointer, stride }, src);
      case Emitter::Var::Type::U32: return walker.emitter.EmitExpr<calyx::Cast<T, u32>>({ Emitter::Var::Type::Pointer, stride }, src);
      case Emitter::Var::Type::I64: return walker.emitter.EmitExpr<calyx::Cast<T, i64>>({ Emitter::Var::Type::Pointer, stride }, src);
      case Emitter::Var::Type::U64: return walker.emitter.EmitExpr<calyx::Cast<T, u64>>({ Emitter::Var::Type::Pointer, stride }, src);
      case Emitter::Var::Type::Float: return walker.emitter.EmitExpr<calyx::Cast<T, float>>({ Emitter::Var::Type::Pointer, stride }, src);
      case Emitter::Var::Type::Double: return walker.emitter.EmitExpr<calyx::Cast<T, double>>({ Emitter::Var::Type::Pointer, stride }, src);
      case Emitter::Var::Type::Pointer: return walker.emitter.EmitExpr<calyx::Cast<T, calyx::Pointer>>({ Emitter::Var::Type::Pointer, stride }, src);
      default: throw std::runtime_error("Bad pointer cast!");
    }
  }
};

template<typename T>
struct ReturnEmitter {
  static void emit_value(ASTWalker& walker, var_index_t src) {
    var_index_t cast = CastToEmitter<T>::emit_value(walker, src);
    walker.emitter.Emit<calyx::Return<calyx::calyx_upcast_t<T>>>(cast);
  }

  static void emit_pointer(ASTWalker& walker, u64 stride, var_index_t src) {
    var_index_t cast = CastToEmitter<T>::emit_pointer(walker, stride, src);
    walker.emitter.Emit<calyx::Return<T>>(cast);
  }
};

}