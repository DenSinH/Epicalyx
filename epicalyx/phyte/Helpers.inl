
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


/*
 * Generic type visitor to be used for different type-dependent operations.
 * One can write their own emitter that handles value types / pointer types / struct types differently.
 * This removes the boilerplate of the rest of the visitor pattern code.
 * */
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
      return src;
    }
    switch (src_t) {
      case calyx::Var::Type::I32: return walker.emitter.EmitExpr<calyx::Cast<T, i32>>({ calyx::Var::Type::Pointer, stride }, src);
      case calyx::Var::Type::U32: return walker.emitter.EmitExpr<calyx::Cast<T, u32>>({ calyx::Var::Type::Pointer, stride }, src);
      case calyx::Var::Type::I64: return walker.emitter.EmitExpr<calyx::Cast<T, i64>>({ calyx::Var::Type::Pointer, stride }, src);
      case calyx::Var::Type::U64: return walker.emitter.EmitExpr<calyx::Cast<T, u64>>({ calyx::Var::Type::Pointer, stride }, src);
      case calyx::Var::Type::Float: return walker.emitter.EmitExpr<calyx::Cast<T, float>>({ calyx::Var::Type::Pointer, stride }, src);
      case calyx::Var::Type::Double: return walker.emitter.EmitExpr<calyx::Cast<T, double>>({ calyx::Var::Type::Pointer, stride }, src);
      case calyx::Var::Type::Pointer: return walker.emitter.EmitExpr<calyx::Cast<T, calyx::Pointer>>({ calyx::Var::Type::Pointer, stride }, src);
      default: throw std::runtime_error("Bad pointer cast!");
    }
  }
};

template<typename T>
struct CallEmitter {
  static calyx::var_index_t emit_value(ASTWalker& walker, calyx::var_index_t fn_idx, calyx::arg_list_t args, calyx::arg_list_t var_args) {
    return walker.emitter.EmitExpr<calyx::Call<calyx::calyx_upcast_t<T>>>({calyx_type_v<calyx::calyx_upcast_t<T>> }, fn_idx, std::move(args), std::move(var_args));
  }

  static calyx::var_index_t emit_pointer(ASTWalker& walker, u64 stride, calyx::var_index_t fn_idx, calyx::arg_list_t args, calyx::arg_list_t var_args) {
    return walker.emitter.EmitExpr<calyx::Call<calyx::Pointer>>({calyx::Var::Type::Pointer, stride }, fn_idx, std::move(args), std::move(var_args));
  }
};

template<typename T>
struct LoadLocalEmitter {
  static calyx::var_index_t emit_value(ASTWalker& walker, calyx::var_index_t c_idx) {
    return walker.emitter.EmitExpr<calyx::LoadLocal<T>>({calyx_type_v<calyx::calyx_upcast_t<T>> }, c_idx);
  }

  static calyx::var_index_t emit_pointer(ASTWalker& walker, u64 stride, calyx::var_index_t c_idx) {
    return walker.emitter.EmitExpr<calyx::LoadLocal<calyx::Pointer>>({calyx::Var::Type::Pointer, stride }, c_idx);
  }
};

template<typename T>
struct LoadLocalAddrEmitter {
  static calyx::var_index_t emit_value(ASTWalker& walker, calyx::var_index_t c_idx) {
    return walker.emitter.EmitExpr<calyx::LoadLocalAddr>({calyx::Var::Type::Pointer, sizeof(T) }, c_idx);
  }

  static calyx::var_index_t emit_pointer(ASTWalker& walker, [[maybe_unused]] u64 stride, calyx::var_index_t c_idx) {
    return walker.emitter.EmitExpr<calyx::LoadLocalAddr>({calyx::Var::Type::Pointer, sizeof(u64) }, c_idx);
  }
};

template<typename T>
struct StoreLocalEmitter {
  static void emit_value(ASTWalker& walker, calyx::var_index_t c_idx, calyx::var_index_t src) {
    calyx::var_index_t cast = CastToEmitter<T>::emit_value(walker, src);
    walker.emitter.Emit<calyx::StoreLocal<T>>(c_idx, cast);
  }

  static void emit_pointer(ASTWalker& walker, u64 stride, calyx::var_index_t c_idx, calyx::var_index_t src) {
    calyx::var_index_t cast = CastToEmitter<T>::emit_pointer(walker, stride, src);
    walker.emitter.Emit<calyx::StoreLocal<T>>(c_idx, cast);
  }
};

template<typename T>
struct LoadGlobalEmitter {
  static calyx::var_index_t emit_value(ASTWalker& walker, const std::string& symbol) {
    return walker.emitter.EmitExpr<calyx::LoadGlobal<T>>({calyx_type_v<calyx::calyx_upcast_t<T>> }, symbol);
  }

  static calyx::var_index_t emit_pointer(ASTWalker& walker, u64 stride, const std::string& symbol) {
    return walker.emitter.EmitExpr<calyx::LoadGlobal<calyx::Pointer>>({calyx::Var::Type::Pointer, stride }, symbol);
  }
};

template<typename T>
struct LoadGlobalAddrEmitter {
  static calyx::var_index_t emit_value(ASTWalker& walker, const std::string& symbol) {
    return walker.emitter.EmitExpr<calyx::LoadGlobalAddr>({calyx::Var::Type::Pointer, sizeof(T) }, symbol);
  }

  static calyx::var_index_t emit_pointer(ASTWalker& walker, [[maybe_unused]] u64 stride, const std::string& symbol) {
    return walker.emitter.EmitExpr<calyx::LoadGlobalAddr>({calyx::Var::Type::Pointer, sizeof(u64) }, symbol);
  }
};

template<typename T>
struct StoreGlobalEmitter {
  static void emit_value(ASTWalker& walker, const std::string& symbol, calyx::var_index_t src) {
    calyx::var_index_t cast = CastToEmitter<T>::emit_value(walker, src);
    walker.emitter.Emit<calyx::StoreGlobal<T>>(symbol, cast);
  }

  static void emit_pointer(ASTWalker& walker, u64 stride, const std::string& symbol, calyx::var_index_t src) {
    calyx::var_index_t cast = CastToEmitter<T>::emit_pointer(walker, stride, src);
    walker.emitter.Emit<calyx::StoreGlobal<T>>(symbol, cast);
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

template<typename T>
struct LoadFromPointerEmitter {
  static calyx::var_index_t emit_value(ASTWalker& walker, calyx::var_index_t ptr_idx) {
    return walker.emitter.EmitExpr<calyx::LoadFromPointer<T>>({calyx_type_v<calyx::calyx_upcast_t<T>> }, ptr_idx);
  }

  static calyx::var_index_t emit_pointer(ASTWalker& walker, u64 stride, calyx::var_index_t ptr_idx) {
    return walker.emitter.EmitExpr<calyx::LoadLocal<calyx::Pointer>>({calyx::Var::Type::Pointer, stride }, ptr_idx);
  }
};

template<typename T>
struct StoreToPointerEmitter {
  static void emit_value(ASTWalker& walker, calyx::var_index_t ptr_idx, calyx::var_index_t src) {
    calyx::var_index_t cast = CastToEmitter<T>::emit_value(walker, src);
    walker.emitter.Emit<calyx::StoreToPointer<T>>(ptr_idx, cast);
  }

  static void emit_pointer(ASTWalker& walker, u64 stride, calyx::var_index_t ptr_idx, calyx::var_index_t src) {
    calyx::var_index_t cast = CastToEmitter<T>::emit_pointer(walker, stride, src);
    walker.emitter.Emit<calyx::StoreToPointer<T>>(ptr_idx, cast);
  }
};


struct ArgumentTypeVisitor : TypeVisitor {
  calyx::Argument result{};

  ArgumentTypeVisitor(calyx::var_index_t arg_idx, bool variadic) {
    result.arg_idx = arg_idx;
    result.variadic = variadic;
  }

  void Visit(const VoidType& type) final { throw std::runtime_error("Incomplete type"); }
  void Visit(const ValueType<i8>& type) final { result.type = calyx::Argument::Type::I8; }
  void Visit(const ValueType<u8>& type) final { result.type = calyx::Argument::Type::U8; }
  void Visit(const ValueType<i16>& type) final { result.type = calyx::Argument::Type::I16; }
  void Visit(const ValueType<u16>& type) final { result.type = calyx::Argument::Type::U16; }
  void Visit(const ValueType<i32>& type) final { result.type = calyx::Argument::Type::I32; }
  void Visit(const ValueType<u32>& type) final { result.type = calyx::Argument::Type::U32; }
  void Visit(const ValueType<i64>& type) final { result.type = calyx::Argument::Type::I64; }
  void Visit(const ValueType<u64>& type) final { result.type = calyx::Argument::Type::U64; }
  void Visit(const ValueType<float>& type) final { result.type = calyx::Argument::Type::Float; }
  void Visit(const ValueType<double>& type) final { result.type = calyx::Argument::Type::Double; }
  void Visit(const PointerType& type) final { result.type = calyx::Argument::Type::Pointer; result.stride = type.Deref()->Sizeof(); }
  void Visit(const ArrayType& type) final { result.type = calyx::Argument::Type::Pointer; result.stride = type.Deref()->Sizeof(); }
  void Visit(const FunctionType& type) final { result.type = calyx::Argument::Type::Pointer; result.stride = 0; }
  void Visit(const StructType& type) final { result.type = calyx::Argument::Type::Struct; result.size = type.Sizeof(); }
  void Visit(const UnionType& type) final { result.type = calyx::Argument::Type::Struct; result.size = type.Sizeof(); }
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
void ASTWalker::EmitPointerIntegralExpr(calyx::Var::Type type, u64 stride, Args... args) {
  switch (type) {
    case calyx::Var::Type::I32: current = emitter.EmitExpr<Op<i32>>({ calyx::Var::Type::Pointer, stride }, args...); break;
    case calyx::Var::Type::U32: current = emitter.EmitExpr<Op<u32>>({ calyx::Var::Type::Pointer, stride }, args...); break;
    case calyx::Var::Type::I64: current = emitter.EmitExpr<Op<i64>>({ calyx::Var::Type::Pointer, stride }, args...); break;
    case calyx::Var::Type::U64: current = emitter.EmitExpr<Op<u64>>({ calyx::Var::Type::Pointer, stride }, args...); break;
    default:
      throw std::runtime_error("Bad type for arithmetic expression");
  }
}

template<template<typename T> class Op, typename... Args>
void ASTWalker::EmitPointerExpr(calyx::Var::Type type, u64 stride, Args... args) {
  switch (type) {
    case calyx::Var::Type::I32: current = emitter.EmitExpr<Op<i32>>({ type, stride }, args...); break;
    case calyx::Var::Type::U32: current = emitter.EmitExpr<Op<u32>>({ type, stride }, args...); break;
    case calyx::Var::Type::I64: current = emitter.EmitExpr<Op<i64>>({ type, stride }, args...); break;
    case calyx::Var::Type::U64: current = emitter.EmitExpr<Op<u64>>({ type, stride }, args...); break;
    case calyx::Var::Type::Float: current = emitter.EmitExpr<Op<float>>({ type, stride }, args...); break;
    case calyx::Var::Type::Double: current = emitter.EmitExpr<Op<double>>({ type, stride }, args...); break;
    case calyx::Var::Type::Pointer: current = emitter.EmitExpr<Op<calyx::Pointer>>({ type, stride }, args...); break;
    default:
      throw std::runtime_error("Bad type for arithmetic expression");
  }
}

template<template<typename T> class Op, typename... Args>
void ASTWalker::EmitCompare(calyx::Var::Type type, Args... args) {
  switch (type) {
    case calyx::Var::Type::I32: current = emitter.EmitExpr<Op<i32>>({ calyx::Var::Type::I32 }, args...); break;
    case calyx::Var::Type::U32: current = emitter.EmitExpr<Op<u32>>({ calyx::Var::Type::I32 }, args...); break;
    case calyx::Var::Type::I64: current = emitter.EmitExpr<Op<i64>>({ calyx::Var::Type::I32 }, args...); break;
    case calyx::Var::Type::U64: current = emitter.EmitExpr<Op<u64>>({ calyx::Var::Type::I32 }, args...); break;
    case calyx::Var::Type::Float: current = emitter.EmitExpr<Op<float>>({ calyx::Var::Type::I32 }, args...); break;
    case calyx::Var::Type::Double: current = emitter.EmitExpr<Op<double>>({ calyx::Var::Type::I32 }, args...); break;
    case calyx::Var::Type::Pointer: current = emitter.EmitExpr<Op<calyx::Pointer>>({ calyx::Var::Type::I32 }, args...); break;
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
  EmitArithExpr<calyx::Binop>(casted.var.type, casted.left, op, casted.right);
}

ASTWalker::BinopCastResult ASTWalker::BinopCastHelper(calyx::var_index_t left, calyx::var_index_t right) {
  auto left_v = emitter.vars[left];
  auto right_v = emitter.vars[right];
  auto left_t = left_v.type;
  auto right_t = right_v.type;
  if (left_t == right_t) {
    return {left_t, left, right};
  }

  if (left_t == calyx::Var::Type::Pointer) {
    switch (right_t) {
      case calyx::Var::Type::I32:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, i32>>(left_v, right);
        return {left_v, left, current};
      case calyx::Var::Type::U32:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, u32>>(left_v, right);
        return {left_v, left, current};
      case calyx::Var::Type::I64:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, i64>>(left_v, right);
        return {left_v, left, current};
      case calyx::Var::Type::U64:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, u64>>(left_v, right);
        return {left_v, left, current};
      case calyx::Var::Type::Float:
      case calyx::Var::Type::Double:
        throw std::runtime_error("Invalid operands for binop: pointer and floating point type");
      case calyx::Var::Type::Pointer:
        // should have been hit before
        throw std::runtime_error("Unreachable");
      default: throw std::runtime_error("Bad binop");
    }
  }

  if (right_t == calyx::Var::Type::Pointer) {
    switch (left_t) {
      case calyx::Var::Type::I32:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, i32>>(right_v, left);
        return {left_v, current, right};
      case calyx::Var::Type::U32:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, u32>>(right_v, left);
        return {left_v, current, right};
      case calyx::Var::Type::I64:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, i64>>(right_v, left);
        return {left_v, current, right};
      case calyx::Var::Type::U64:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, u64>>(right_v, left);
        return {left_v, current, right};
      case calyx::Var::Type::Float:
      case calyx::Var::Type::Double:
        throw std::runtime_error("Invalid operands for binop: floating point type and pointer");
      default: throw std::runtime_error("Bad binop");
    }
  }

  switch (left_t) {
    case calyx::Var::Type::I32: {
      switch (right_t) {
        case calyx::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<u32, i32>>({ right_t }, left);
          return {{right_t}, current, right};
        case calyx::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<i64, i32>>({ right_t }, left);
          return {{right_t}, current, right};
        case calyx::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<u64, i32>>({ right_t }, left);
          return {{right_t}, current, right};
        case calyx::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, i32>>({ right_t }, left);
          return {{right_t}, current, right};
        case calyx::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, i32>>({ right_t }, left);
          return {{right_t}, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case calyx::Var::Type::U32: {
      switch (right_t) {
        case calyx::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<u32, i32>>({ left_t }, right);
          return {{left_t}, left, current};
        case calyx::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<i64, u32>>({ right_t }, left);
          return {{right_t}, current, right};
        case calyx::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<u64, u32>>({ right_t }, left);
          return {{right_t}, current, right};
        case calyx::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, u32>>({ right_t }, left);
          return {{right_t}, current, right};
        case calyx::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, u32>>({ right_t }, left);
          return {{right_t}, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case calyx::Var::Type::I64: {
      switch (right_t) {
        case calyx::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<i64, i32>>({ left_t }, right);
          return {{left_t}, left, current};
        case calyx::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<i64, u32>>({ left_t }, right);
          return {{left_t}, left, current};
        case calyx::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<u64, i64>>({ right_t }, left);
          return {{right_t}, current, right};
        case calyx::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, i64>>({ right_t }, left);
          return {{right_t}, current, right};
        case calyx::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, i64>>({ right_t }, left);
          return {{right_t}, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case calyx::Var::Type::U64: {
      switch (right_t) {
        case calyx::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<u64, i32>>({left_t}, right);
          return {{left_t}, left, current};
        case calyx::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<u64, u32>>({left_t}, right);
          return {{left_t}, left, current};
        case calyx::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<u64, i64>>({left_t}, right);
          return {{left_t}, left, current};
        case calyx::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, u64>>({right_t}, left);
          return {{right_t}, current, right};
        case calyx::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, u64>>({right_t}, left);
          return {{right_t}, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case calyx::Var::Type::Float: {
      switch (right_t) {
        case calyx::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, float>>({right_t}, left);
          return {{right_t}, current, right};
        case calyx::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<float, i32>>({left_t}, right);
          return {{left_t}, left, current};
        case calyx::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<float, u32>>({left_t}, right);
          return {{left_t}, left, current};
        case calyx::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<float, i64>>({left_t}, right);
          return {{left_t}, left, current};
        case calyx::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<float, u64>>({left_t}, right);
          return {{left_t}, left, current};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case calyx::Var::Type::Double: {
      switch (right_t) {
        case calyx::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<double, i32>>({left_t}, right);
          return {{left_t}, left, current};
        case calyx::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<double, u32>>({left_t}, right);
          return {{left_t}, left, current};
        case calyx::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<double, i64>>({left_t}, right);
          return {{left_t}, left, current};
        case calyx::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<double, u64>>({left_t}, right);
          return {{left_t}, left, current};
        case calyx::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<double, float>>({left_t}, right);
          return {{left_t}, left, current};
        default: throw std::runtime_error("Bad binop");
      }
    }
    default: throw std::runtime_error("Bad binop");
  }
}

}