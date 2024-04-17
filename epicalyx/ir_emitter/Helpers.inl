
namespace epi {

using namespace ast;

namespace detail {

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
struct EmitterTypeVisitor : TypeVisitor {
  using args_t = func_args_t<decltype(emit<i32>::emit_value)>;
  using return_t = func_return_t<decltype(emit<i32>::emit_value)>;

  EmitterTypeVisitor(ASTWalker& walker, args_t&& args) :
          walker(walker), args{std::move(args)} {

  }

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
  void Visit(const StructType& type) final { throw cotyl::UnimplementedException(); }
  void Visit(const UnionType& type) final { throw cotyl::UnimplementedException(); }
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
struct CallEmitter {
  static var_index_t emit_value(ASTWalker& walker, var_index_t fn_idx, calyx::arg_list_t args, calyx::arg_list_t var_args) {
    return walker.emitter.EmitExpr<calyx::Call<calyx::calyx_upcast_t<T>>>({calyx_var_type_v<calyx::calyx_upcast_t<T>> }, fn_idx, std::move(args), std::move(var_args));
  }

  static var_index_t emit_pointer(ASTWalker& walker, u64 stride, var_index_t fn_idx, calyx::arg_list_t args, calyx::arg_list_t var_args) {
    return walker.emitter.EmitExpr<calyx::Call<calyx::Pointer>>({Emitter::Var::Type::Pointer, stride }, fn_idx, std::move(args), std::move(var_args));
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


struct ArgumentTypeVisitor : TypeVisitor {
  calyx::Argument result{};

  ArgumentTypeVisitor(var_index_t arg_idx, bool variadic) {
    result.arg_idx = arg_idx;
    result.variadic = variadic;
  }

  void Visit(const VoidType& type) final { throw std::runtime_error("Incomplete type"); }
  void Visit(const ValueType<i8>& type) final { result.type = calyx::Local::Type::I8; }
  void Visit(const ValueType<u8>& type) final { result.type = calyx::Local::Type::U8; }
  void Visit(const ValueType<i16>& type) final { result.type = calyx::Local::Type::I16; }
  void Visit(const ValueType<u16>& type) final { result.type = calyx::Local::Type::U16; }
  void Visit(const ValueType<i32>& type) final { result.type = calyx::Local::Type::I32; }
  void Visit(const ValueType<u32>& type) final { result.type = calyx::Local::Type::U32; }
  void Visit(const ValueType<i64>& type) final { result.type = calyx::Local::Type::I64; }
  void Visit(const ValueType<u64>& type) final { result.type = calyx::Local::Type::U64; }
  void Visit(const ValueType<float>& type) final { result.type = calyx::Local::Type::Float; }
  void Visit(const ValueType<double>& type) final { result.type = calyx::Local::Type::Double; }
  void Visit(const PointerType& type) final { result.type = calyx::Local::Type::Pointer; result.stride = type.Deref()->Sizeof(); }
  void Visit(const ArrayType& type) final { result.type = calyx::Local::Type::Pointer; result.stride = type.Deref()->Sizeof(); }
  void Visit(const FunctionType& type) final { result.type = calyx::Local::Type::Pointer; result.stride = 0; }
  void Visit(const StructType& type) final { result.type = calyx::Local::Type::Struct; result.size = type.Sizeof(); }
  void Visit(const UnionType& type) final { result.type = calyx::Local::Type::Struct; result.size = type.Sizeof(); }
};


struct GlobalInitializerVisitor : TypeVisitor {
  calyx::global_t result{};

  void Visit(const VoidType& type) final { throw std::runtime_error("Incomplete type for global initializer"); }
  void Visit(const ValueType<i8>& type) final { result = i8{0}; }
  void Visit(const ValueType<u8>& type) final { result = u8{0}; }
  void Visit(const ValueType<i16>& type) final { result = i16{0}; }
  void Visit(const ValueType<u16>& type) final { result = u16{0}; }
  void Visit(const ValueType<i32>& type) final { result = i32{0}; }
  void Visit(const ValueType<u32>& type) final { result = u32{0}; }
  void Visit(const ValueType<i64>& type) final { result = i64{0}; }
  void Visit(const ValueType<u64>& type) final { result = u64{0}; }
  void Visit(const ValueType<float>& type) final { result = float{0}; }
  void Visit(const ValueType<double>& type) final { result = double{0}; }
  void Visit(const PointerType& type) final { result = calyx::Pointer{0}; }
  void Visit(const ArrayType& type) final { throw cotyl::UnimplementedException("global array"); }
  void Visit(const FunctionType& type) final { result = calyx::Pointer{0}; }
  void Visit(const StructType& type) final { throw cotyl::UnimplementedException("global struct"); }
  void Visit(const UnionType& type) final { throw cotyl::UnimplementedException("global struct");  }
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
      throw std::runtime_error("Bad type for integral expression");
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
      throw std::runtime_error("Bad type for arithmetic expression");
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
      throw std::runtime_error("Bad type for arithmetic expression");
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
      throw std::runtime_error("Bad type for arithmetic expression");
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
      throw std::runtime_error("Bad type for branch expression");
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
      throw std::runtime_error("Bad type for branch expression");
  }
}

void ASTWalker::BinopHelper(var_index_t left, calyx::BinopType op, var_index_t right) {
  auto casted = BinopCastHelper(left, right);
  EmitArithExpr<calyx::Binop>(casted.var.type, casted.left, op, casted.right);
}

ASTWalker::BinopCastResult ASTWalker::BinopCastHelper(var_index_t left, var_index_t right) {
  auto left_v = emitter.vars[left];
  auto right_v = emitter.vars[right];
  auto left_t = left_v.type;
  auto right_t = right_v.type;
  if (left_t == right_t) {
    return {left_t, left, right};
  }

  if (left_t == Emitter::Var::Type::Pointer) {
    switch (right_t) {
      case Emitter::Var::Type::I32:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, i32>>(left_v, right);
        return {left_v, left, current};
      case Emitter::Var::Type::U32:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, u32>>(left_v, right);
        return {left_v, left, current};
      case Emitter::Var::Type::I64:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, i64>>(left_v, right);
        return {left_v, left, current};
      case Emitter::Var::Type::U64:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, u64>>(left_v, right);
        return {left_v, left, current};
      case Emitter::Var::Type::Float:
      case Emitter::Var::Type::Double:
        throw std::runtime_error("Invalid operands for binop: pointer and floating point type");
      case Emitter::Var::Type::Pointer:
        // should have been hit before
        throw std::runtime_error("Unreachable");
      default: throw std::runtime_error("Bad binop");
    }
  }

  if (right_t == Emitter::Var::Type::Pointer) {
    switch (left_t) {
      case Emitter::Var::Type::I32:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, i32>>(right_v, left);
        return {left_v, current, right};
      case Emitter::Var::Type::U32:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, u32>>(right_v, left);
        return {left_v, current, right};
      case Emitter::Var::Type::I64:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, i64>>(right_v, left);
        return {left_v, current, right};
      case Emitter::Var::Type::U64:
        current = emitter.EmitExpr<calyx::Cast<calyx::Pointer, u64>>(right_v, left);
        return {left_v, current, right};
      case Emitter::Var::Type::Float:
      case Emitter::Var::Type::Double:
        throw std::runtime_error("Invalid operands for binop: floating point type and pointer");
      default: throw std::runtime_error("Bad binop");
    }
  }

  switch (left_t) {
    case Emitter::Var::Type::I32: {
      switch (right_t) {
        case Emitter::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<u32, i32>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<i64, i32>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<u64, i32>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, i32>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, i32>>({ right_t }, left);
          return {{right_t}, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case Emitter::Var::Type::U32: {
      switch (right_t) {
        case Emitter::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<u32, i32>>({ left_t }, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<i64, u32>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<u64, u32>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, u32>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, u32>>({ right_t }, left);
          return {{right_t}, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case Emitter::Var::Type::I64: {
      switch (right_t) {
        case Emitter::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<i64, i32>>({ left_t }, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<i64, u32>>({ left_t }, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<u64, i64>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, i64>>({ right_t }, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, i64>>({ right_t }, left);
          return {{right_t}, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case Emitter::Var::Type::U64: {
      switch (right_t) {
        case Emitter::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<u64, i32>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<u64, u32>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<u64, i64>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<float, u64>>({right_t}, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, u64>>({right_t}, left);
          return {{right_t}, current, right};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case Emitter::Var::Type::Float: {
      switch (right_t) {
        case Emitter::Var::Type::Double:
          current = emitter.EmitExpr<calyx::Cast<double, float>>({right_t}, left);
          return {{right_t}, current, right};
        case Emitter::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<float, i32>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<float, u32>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<float, i64>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<float, u64>>({left_t}, right);
          return {{left_t}, left, current};
        default: throw std::runtime_error("Bad binop");
      }
    }
    case Emitter::Var::Type::Double: {
      switch (right_t) {
        case Emitter::Var::Type::I32:
          current = emitter.EmitExpr<calyx::Cast<double, i32>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::U32:
          current = emitter.EmitExpr<calyx::Cast<double, u32>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::I64:
          current = emitter.EmitExpr<calyx::Cast<double, i64>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::U64:
          current = emitter.EmitExpr<calyx::Cast<double, u64>>({left_t}, right);
          return {{left_t}, left, current};
        case Emitter::Var::Type::Float:
          current = emitter.EmitExpr<calyx::Cast<double, float>>({left_t}, right);
          return {{left_t}, left, current};
        default: throw std::runtime_error("Bad binop");
      }
    }
    default: throw std::runtime_error("Bad binop");
  }
}

}