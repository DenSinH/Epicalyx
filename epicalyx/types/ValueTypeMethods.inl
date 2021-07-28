
#define VALUE_TYPE_INTEGRAL_TYPE_HANDLERS(MACRO, ...) \
    MACRO(__VA_ARGS__, u8) \
    MACRO(__VA_ARGS__, i8) \
    MACRO(__VA_ARGS__, u16) \
    MACRO(__VA_ARGS__, i16) \
    MACRO(__VA_ARGS__, u32) \
    MACRO(__VA_ARGS__, i32) \
    MACRO(__VA_ARGS__, u64) \
    MACRO(__VA_ARGS__, i64) \

#define VALUE_TYPE_NUMERIC_TYPE_HANDLERS(MACRO, ...) \
    VALUE_TYPE_INTEGRAL_TYPE_HANDLERS(MACRO, __VA_ARGS__) \
    MACRO(__VA_ARGS__, float) \
    MACRO(__VA_ARGS__, double) \

#define VALUE_TYPE_BINOP_HANDLER(handler, functional, type) \
template<typename T> \
inline BINOP_HANDLER(ValueType<T>::R ## handler, ValueType<type>) { \
  using common_t = std::common_type_t<T, type>; \
  return ValueTypeRBinOp<type, functional>(other); \
}

#define INTEGRAL_VALUE_TYPE_BINOP_HANDLER(handler, functional, type) \
template<typename T> \
inline BINOP_HANDLER(ValueType<T>::R ## handler, ValueType<type>) { \
  if constexpr(!std::is_integral_v<T>) { \
    throw std::runtime_error("Non-integral argument to integral operator"); \
  } \
  else { \
    using common_t = std::common_type_t<T, type>; \
    return ValueTypeRBinOp<type, functional>(other); \
  } \
}

#define VALUE_TYPE_BOOL_BINOP_HANDLER(handler, functional, type) \
template<typename T> \
inline BINOP_HANDLER(ValueType<T>::R ## handler, ValueType<type>) { \
  using common_t = std::common_type_t<T, type>; \
  return ValueTypeRBoolBinOp<type, functional>(other); \
}

template<typename T>
template<typename L, template<typename t> class _handler, typename common_t>
pType<> ValueType<T>::ValueTypeRBinOp(const ValueType<L>& other) const {
  // results of binary expressions are never an lvalue
  _handler<common_t> handler;
  if (HasValue() && other.HasValue()) {
    return MakeType<ValueType<common_t>>(handler(other.Get(), Get()), LValueNess::None, 0);
  }
  return MakeType<ValueType<common_t>>(LValueNess::None, 0);
}

template<typename T>
template<typename L, template<typename t> class _handler, typename common_t>
pType<> ValueType<T>::ValueTypeRBoolBinOp(const ValueType<L>& other) const {
  _handler<common_t> handler;
  if (HasValue() && other.HasValue()) {
    return MakeBool(handler(other.Get(), Get()));
  }
  return MakeBool();
}

template<typename T>
inline BINOP_HANDLER(ValueType<T>::RSub, PointerType) {
  auto same_type = other.Clone();
  same_type->ForgetConstInfo();
  same_type->lvalue = LValueNess::None;
  return same_type;
}

template<typename T>
inline BINOP_HANDLER(ValueType<T>::RAdd, PointerType) {
  auto same_type = other.Clone();
  same_type->ForgetConstInfo();
  same_type->lvalue = LValueNess::None;
  return same_type;
}

template<typename T>
inline BINOP_HANDLER(ValueType<T>::RCommonType, PointerType) {
  return other.Clone();
}

template<typename T>
inline UNOP_HANDLER(ValueType<T>::Pos) {
  auto same_type = Clone();
  same_type->lvalue = LValueNess::None;
  return same_type;
}

template<typename T>
inline UNOP_HANDLER(ValueType<T>::Neg) {
  if (HasValue()) {
      return MakeType<ValueType<T>>(-Get(), LValueNess::None, 0);
  }
  return MakeType<ValueType<T>>(LValueNess::None, 0);
}

template<typename T>
inline UNOP_HANDLER(ValueType<T>::BinNot) {
  if constexpr(!std::is_integral_v<T>) {
    throw std::runtime_error("Cannot perform binary operations on floating point types");
  }
  else {
    if (HasValue()) {
      return MakeType<ValueType<T>>(~Get(), LValueNess::None, 0);
    }
    return MakeType<ValueType<T>>(LValueNess::None, 0);
  }
}


template<typename T> BINOP_HANDLER(ValueType<T>::RCommonType, ValueType<i8>) { return MakeType<ValueType<std::common_type_t<T, i8>>>(LValueNess::None); }
template<typename T> BINOP_HANDLER(ValueType<T>::RCommonType, ValueType<u8>) { return MakeType<ValueType<std::common_type_t<T, u8>>>(LValueNess::None); }
template<typename T> BINOP_HANDLER(ValueType<T>::RCommonType, ValueType<u16>) { return MakeType<ValueType<std::common_type_t<T, u16>>>(LValueNess::None); }
template<typename T> BINOP_HANDLER(ValueType<T>::RCommonType, ValueType<i16>) { return MakeType<ValueType<std::common_type_t<T, i16>>>(LValueNess::None); }
template<typename T> BINOP_HANDLER(ValueType<T>::RCommonType, ValueType<u32>) { return MakeType<ValueType<std::common_type_t<T, u32>>>(LValueNess::None); }
template<typename T> BINOP_HANDLER(ValueType<T>::RCommonType, ValueType<i32>) { return MakeType<ValueType<std::common_type_t<T, i32>>>(LValueNess::None); }
template<typename T> BINOP_HANDLER(ValueType<T>::RCommonType, ValueType<u64>) { return MakeType<ValueType<std::common_type_t<T, u64>>>(LValueNess::None); }
template<typename T> BINOP_HANDLER(ValueType<T>::RCommonType, ValueType<i64>) { return MakeType<ValueType<std::common_type_t<T, i64>>>(LValueNess::None); }
template<typename T> BINOP_HANDLER(ValueType<T>::RCommonType, ValueType<float>) { return MakeType<ValueType<std::common_type_t<T, float>>>(LValueNess::None); }
template<typename T> BINOP_HANDLER(ValueType<T>::RCommonType, ValueType<double>) { return MakeType<ValueType<std::common_type_t<T, double>>>(LValueNess::None); }

VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BINOP_HANDLER, Add, std::plus)
VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BINOP_HANDLER, Sub, std::minus)

VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BINOP_HANDLER, Mul, std::multiplies)
VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BINOP_HANDLER, Div, std::divides)
VALUE_TYPE_INTEGRAL_TYPE_HANDLERS(INTEGRAL_VALUE_TYPE_BINOP_HANDLER, Mod, std::modulus)

template<typename T> struct lshift { T operator()(const T& l, const T& r) const { return l << r; }};
template<typename T> struct rshift { T operator()(const T& l, const T& r) const { return l << r; }};

VALUE_TYPE_INTEGRAL_TYPE_HANDLERS(INTEGRAL_VALUE_TYPE_BINOP_HANDLER, LShift, lshift)
VALUE_TYPE_INTEGRAL_TYPE_HANDLERS(INTEGRAL_VALUE_TYPE_BINOP_HANDLER, RShift, rshift)

VALUE_TYPE_INTEGRAL_TYPE_HANDLERS(INTEGRAL_VALUE_TYPE_BINOP_HANDLER, Xor, std::bit_xor)
VALUE_TYPE_INTEGRAL_TYPE_HANDLERS(INTEGRAL_VALUE_TYPE_BINOP_HANDLER, BinAnd, std::bit_and)
VALUE_TYPE_INTEGRAL_TYPE_HANDLERS(INTEGRAL_VALUE_TYPE_BINOP_HANDLER, BinOr, std::bit_or)

template<typename T> struct lt { T operator()(const T& l, const T& r) const { return l < r; }};
template<typename T> struct eq { T operator()(const T& l, const T& r) const { return l == r; }};

VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BOOL_BINOP_HANDLER, Lt, lt)
VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BOOL_BINOP_HANDLER, Eq, eq)

template<typename T>
inline BINOP_HANDLER(ValueType<T>::REq, PointerType) {
  if constexpr(std::is_integral_v<T>) {
    return MakeBool();
  }
  throw std::runtime_error("Cannot compare pointers with non-integral types");
}

template<typename T>
inline BINOP_HANDLER(ValueType<T>::RLt, PointerType) {
  if constexpr(std::is_integral_v<T>) {
    return MakeBool();
  }
  throw std::runtime_error("Cannot compare pointers with non-integral types");
}