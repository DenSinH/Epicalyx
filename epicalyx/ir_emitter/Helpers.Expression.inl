#include "Default.h"
#include "Decltype.h"


namespace epi {

using namespace ast;

namespace detail {

template<typename T>
struct CallEmitter {
  static var_index_t emit_value(ASTWalker& walker, var_index_t fn_idx, calyx::ArgData args) {
    return walker.emitter.EmitExpr<calyx::Call<calyx::calyx_upcast_t<T>>>({calyx_var_type_v<calyx::calyx_upcast_t<T>> }, fn_idx, std::move(args));
  }

  static var_index_t emit_pointer(ASTWalker& walker, u64 stride, var_index_t fn_idx, calyx::ArgData args) {
    return walker.emitter.EmitExpr<calyx::Call<calyx::Pointer>>({ Emitter::Var::Type::Pointer, stride }, fn_idx, std::move(args));
  }
};

template<typename T>
struct LoadLocalEmitter {
  static var_index_t emit_value(ASTWalker& walker, var_index_t c_idx) {
    return walker.emitter.EmitExpr<calyx::LoadLocal<T>>({ calyx_var_type_v<typename calyx::LoadLocal<T>::result_t> }, c_idx);
  }

  static var_index_t emit_pointer(ASTWalker& walker, u64 stride, var_index_t c_idx) {
    return walker.emitter.EmitExpr<calyx::LoadLocal<calyx::Pointer>>({Emitter::Var::Type::Pointer, stride }, c_idx);
  }
};

template<typename T>
struct LoadLocalAddrEmitter {
  static var_index_t emit_value(ASTWalker& walker, var_index_t c_idx) {
    return walker.emitter.EmitExpr<calyx::LoadLocalAddr>({Emitter::Var::Type::Pointer, sizeof(T) }, c_idx);
  }

  static var_index_t emit_pointer(ASTWalker& walker, [[maybe_unused]] u64 stride, var_index_t c_idx) {
    return walker.emitter.EmitExpr<calyx::LoadLocalAddr>({Emitter::Var::Type::Pointer, sizeof(u64) }, c_idx);
  }
};

template<typename T>
struct StoreLocalEmitter {
  static void emit_value(ASTWalker& walker, var_index_t c_idx, var_index_t src) {
    var_index_t cast = CastToEmitter<T>::emit_value(walker, src);
    walker.emitter.Emit<calyx::StoreLocal<T>>(c_idx, cast);
  }

  static void emit_pointer(ASTWalker& walker, u64 stride, var_index_t c_idx, var_index_t src) {
    var_index_t cast = CastToEmitter<T>::emit_pointer(walker, stride, src);
    walker.emitter.Emit<calyx::StoreLocal<T>>(c_idx, cast);
  }
};

template<typename T>
struct LoadGlobalEmitter {
  static var_index_t emit_value(ASTWalker& walker, cotyl::CString symbol) {
    return walker.emitter.EmitExpr<calyx::LoadGlobal<T>>({calyx_var_type_v<typename calyx::LoadGlobal<T>::result_t> }, std::move(symbol));
  }

  static var_index_t emit_pointer(ASTWalker& walker, u64 stride, cotyl::CString symbol) {
    return walker.emitter.EmitExpr<calyx::LoadGlobal<calyx::Pointer>>({Emitter::Var::Type::Pointer, stride }, std::move(symbol));
  }
};

template<typename T>
struct LoadGlobalAddrEmitter {
  static var_index_t emit_value(ASTWalker& walker, cotyl::CString symbol) {
    return walker.emitter.EmitExpr<calyx::LoadGlobalAddr>({Emitter::Var::Type::Pointer, sizeof(T) }, std::move(symbol));
  }

  static var_index_t emit_pointer(ASTWalker& walker, [[maybe_unused]] u64 stride, cotyl::CString symbol) {
    return walker.emitter.EmitExpr<calyx::LoadGlobalAddr>({Emitter::Var::Type::Pointer, sizeof(u64) }, std::move(symbol));
  }
};

template<typename T>
struct StoreGlobalEmitter {
  static void emit_value(ASTWalker& walker, cotyl::CString symbol, var_index_t src) {
    var_index_t cast = CastToEmitter<T>::emit_value(walker, src);
    walker.emitter.Emit<calyx::StoreGlobal<T>>(std::move(symbol), cast);
  }

  static void emit_pointer(ASTWalker& walker, u64 stride, cotyl::CString symbol, var_index_t src) {
    var_index_t cast = CastToEmitter<T>::emit_pointer(walker, stride, src);
    walker.emitter.Emit<calyx::StoreGlobal<T>>(std::move(symbol), cast);
  }
};

template<typename T>
struct LoadFromPointerEmitter {
  static var_index_t emit_value(ASTWalker& walker, var_index_t ptr_idx) {
    return walker.emitter.EmitExpr<calyx::LoadFromPointer<T>>({calyx_var_type_v<typename calyx::LoadFromPointer<T>::result_t> }, ptr_idx);
  }

  static var_index_t emit_pointer(ASTWalker& walker, u64 stride, var_index_t ptr_idx) {
    return walker.emitter.EmitExpr<calyx::LoadFromPointer<calyx::Pointer>>({Emitter::Var::Type::Pointer, stride }, ptr_idx);
  }
};

template<typename T>
struct StoreToPointerEmitter {
  static void emit_value(ASTWalker& walker, var_index_t ptr_idx, var_index_t src) {
    var_index_t cast = CastToEmitter<T>::emit_value(walker, src);
    walker.emitter.Emit<calyx::StoreToPointer<T>>(ptr_idx, cast);
  }

  static void emit_pointer(ASTWalker& walker, u64 stride, var_index_t ptr_idx, var_index_t src) {
    var_index_t cast = CastToEmitter<T>::emit_pointer(walker, stride, src);
    walker.emitter.Emit<calyx::StoreToPointer<T>>(ptr_idx, cast);
  }
};

}

template<template<typename T> class Op, typename... Args>
void ASTWalker::EmitIntegralExpr(Emitter::Var::Type type, Args... args) {
  switch (type) {
    case Emitter::Var::Type::I32: current = emitter.EmitExpr<Op<i32>>({ type }, args...); break;
    case Emitter::Var::Type::U32: current = emitter.EmitExpr<Op<u32>>({ type }, args...); break;
    case Emitter::Var::Type::I64: current = emitter.EmitExpr<Op<i64>>({ type }, args...); break;
    case Emitter::Var::Type::U64: current = emitter.EmitExpr<Op<u64>>({ type }, args...); break;
    default:
      throw EmitterError("Bad type for integral expression");
  }
}

template<template<typename T> class Op, typename... Args>
void ASTWalker::EmitArithExpr(Emitter::Var::Type type, Args... args) {
  switch (type) {
    case Emitter::Var::Type::I32: current = emitter.EmitExpr<Op<i32>>({ type }, args...); break;
    case Emitter::Var::Type::U32: current = emitter.EmitExpr<Op<u32>>({ type }, args...); break;
    case Emitter::Var::Type::I64: current = emitter.EmitExpr<Op<i64>>({ type }, args...); break;
    case Emitter::Var::Type::U64: current = emitter.EmitExpr<Op<u64>>({ type }, args...); break;
    case Emitter::Var::Type::Float: current = emitter.EmitExpr<Op<float>>({ type }, args...); break;
    case Emitter::Var::Type::Double: current = emitter.EmitExpr<Op<double>>({ type }, args...); break;
    default:
      throw EmitterError("Bad type for arithmetic expression");
  }
}

template<template<typename T> class Op, typename... Args>
void ASTWalker::EmitPointerIntegralExpr(Emitter::Var::Type type, u64 stride, Args... args) {
  switch (type) {
    case Emitter::Var::Type::I32: current = emitter.EmitExpr<Op<i32>>({ Emitter::Var::Type::Pointer, stride }, args...); break;
    case Emitter::Var::Type::U32: current = emitter.EmitExpr<Op<u32>>({ Emitter::Var::Type::Pointer, stride }, args...); break;
    case Emitter::Var::Type::I64: current = emitter.EmitExpr<Op<i64>>({ Emitter::Var::Type::Pointer, stride }, args...); break;
    case Emitter::Var::Type::U64: current = emitter.EmitExpr<Op<u64>>({ Emitter::Var::Type::Pointer, stride }, args...); break;
    default:
      throw EmitterError("Bad type for arithmetic expression");
  }
}

template<template<typename T> class Op, typename... Args>
void ASTWalker::EmitPointerExpr(Emitter::Var::Type type, u64 stride, Args... args) {
  switch (type) {
    case Emitter::Var::Type::I32: current = emitter.EmitExpr<Op<i32>>({ type, stride }, args...); break;
    case Emitter::Var::Type::U32: current = emitter.EmitExpr<Op<u32>>({ type, stride }, args...); break;
    case Emitter::Var::Type::I64: current = emitter.EmitExpr<Op<i64>>({ type, stride }, args...); break;
    case Emitter::Var::Type::U64: current = emitter.EmitExpr<Op<u64>>({ type, stride }, args...); break;
    case Emitter::Var::Type::Float: current = emitter.EmitExpr<Op<float>>({ type, stride }, args...); break;
    case Emitter::Var::Type::Double: current = emitter.EmitExpr<Op<double>>({ type, stride }, args...); break;
    case Emitter::Var::Type::Pointer: current = emitter.EmitExpr<Op<calyx::Pointer>>({ type, stride }, args...); break;
    default:
      throw EmitterError("Bad type for pointer expression");
  }
}

template<template<typename T> class Op, typename... Args>
void ASTWalker::EmitCompare(Emitter::Var::Type type, Args... args) {
  switch (type) {
    case Emitter::Var::Type::I32: current = emitter.EmitExpr<Op<i32>>({ Emitter::Var::Type::I32 }, args...); break;
    case Emitter::Var::Type::U32: current = emitter.EmitExpr<Op<u32>>({ Emitter::Var::Type::I32 }, args...); break;
    case Emitter::Var::Type::I64: current = emitter.EmitExpr<Op<i64>>({ Emitter::Var::Type::I32 }, args...); break;
    case Emitter::Var::Type::U64: current = emitter.EmitExpr<Op<u64>>({ Emitter::Var::Type::I32 }, args...); break;
    case Emitter::Var::Type::Float: current = emitter.EmitExpr<Op<float>>({ Emitter::Var::Type::I32 }, args...); break;
    case Emitter::Var::Type::Double: current = emitter.EmitExpr<Op<double>>({ Emitter::Var::Type::I32 }, args...); break;
    case Emitter::Var::Type::Pointer: current = emitter.EmitExpr<Op<calyx::Pointer>>({ Emitter::Var::Type::I32 }, args...); break;
    default:
      throw EmitterError("Bad type for branch expression");
  }
}

template<template<typename T> class Op, typename... Args>
void ASTWalker::EmitBranch(Emitter::Var::Type type, Args... args) {
  switch (type) {
    case Emitter::Var::Type::I32: emitter.Emit<Op<i32>>(args...); break;
    case Emitter::Var::Type::U32: emitter.Emit<Op<u32>>(args...); break;
    case Emitter::Var::Type::I64: emitter.Emit<Op<i64>>(args...); break;
    case Emitter::Var::Type::U64: emitter.Emit<Op<u64>>(args...); break;
    case Emitter::Var::Type::Float: emitter.Emit<Op<float>>(args...); break;
    case Emitter::Var::Type::Double: emitter.Emit<Op<double>>(args...); break;
    case Emitter::Var::Type::Pointer: emitter.Emit<Op<calyx::Pointer>>(args...); break;
    default:
      throw EmitterError("Bad type for branch expression");
  }
}

}