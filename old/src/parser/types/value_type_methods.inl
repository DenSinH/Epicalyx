
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
    return ValueTypeRBinOp<type>(other, functional); \
}

#define INTEGRAL_VALUE_TYPE_BINOP_HANDLER(handler, functional, type) \
template<typename T> \
inline BINOP_HANDLER(ValueType<T>::R ## handler, ValueType<type>) { \
    if constexpr(!std::is_integral_v<T>) { \
        throw std::runtime_error("Non-integral argument to integral operator"); \
    } \
    else { \
        using common_t = std::common_type_t<T, type>; \
        return ValueTypeRBinOp<type>(other, functional); \
    } \
}

#define VALUE_TYPE_BOOL_BINOP_HANDLER(handler, functional, type) \
template<typename T> \
inline BINOP_HANDLER(ValueType<T>::R ## handler, ValueType<type>) { \
    using common_t = std::common_type_t<T, type>; \
    return ValueTypeRBoolBinOp<type>(other, functional); \
}

template<typename T>
inline BINOP_HANDLER(ValueType<T>::RSub, PointerType) {
    auto same_type = other.Clone();
    same_type->ForgetConstInfo();
    same_type->SetNotLValue();
    return same_type;
}

template<typename T>
inline BINOP_HANDLER(ValueType<T>::RAdd, PointerType) {
    auto same_type = other.Clone();
    same_type->ForgetConstInfo();
    same_type->SetNotLValue();
    return same_type;
}

template<typename T>
inline UNOP_HANDLER(ValueType<T>::Pos) {
    auto same_type = Clone();
    same_type->SetNotLValue();
    return same_type;
}

template<typename T>
inline UNOP_HANDLER(ValueType<T>::Neg) {
    if (HasValue()) {
        return MAKE_TYPE(ValueType<T>)(-Get(), LValueNess::None, 0);
    }
    return MAKE_TYPE(ValueType<T>)(LValueNess::None, 0);
}

template<typename T>
inline UNOP_HANDLER(ValueType<T>::BinNot) {
    if constexpr(!std::is_integral_v<T>) {
        throw std::runtime_error("Cannot perform binary operations on floating point types");
    }
    else {
        if (HasValue()) {
            return MAKE_TYPE(ValueType<T>)(~Get(), LValueNess::None, 0);
        }
        return MAKE_TYPE(ValueType<T>)(LValueNess::None, 0);
    }
}

VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BINOP_HANDLER, Add, std::plus<common_t>())
VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BINOP_HANDLER, Sub, std::minus<common_t>())

VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BINOP_HANDLER, Mul, std::multiplies<common_t>())
VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BINOP_HANDLER, Div, std::divides<common_t>())
VALUE_TYPE_INTEGRAL_TYPE_HANDLERS(INTEGRAL_VALUE_TYPE_BINOP_HANDLER, Mod, std::modulus<common_t>())

VALUE_TYPE_INTEGRAL_TYPE_HANDLERS(INTEGRAL_VALUE_TYPE_BINOP_HANDLER, LShift, [](common_t l, common_t r) -> common_t { return l << r; } )
VALUE_TYPE_INTEGRAL_TYPE_HANDLERS(INTEGRAL_VALUE_TYPE_BINOP_HANDLER, RShift, [](common_t l, common_t r) -> common_t { return l >> r; })

VALUE_TYPE_INTEGRAL_TYPE_HANDLERS(INTEGRAL_VALUE_TYPE_BINOP_HANDLER, Xor, std::bit_xor<common_t>())
VALUE_TYPE_INTEGRAL_TYPE_HANDLERS(INTEGRAL_VALUE_TYPE_BINOP_HANDLER, BinAnd, std::bit_and<common_t>())
VALUE_TYPE_INTEGRAL_TYPE_HANDLERS(INTEGRAL_VALUE_TYPE_BINOP_HANDLER, BinOr, std::bit_or<common_t>())

VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BOOL_BINOP_HANDLER, Lt, [](common_t l, common_t r) -> i32 { return l < r; })
VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BOOL_BINOP_HANDLER, Gt, [](common_t l, common_t r) -> i32 { return l < r; })
VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BOOL_BINOP_HANDLER, Eq, [](common_t l, common_t r) -> i32 { return l == r; })

template<typename T>
inline BINOP_HANDLER(ValueType<T>::REq, PointerType) {
    if constexpr(std::is_integral_v<T>) {
        return MakeBool();
    }
    throw std::runtime_error("Cannot compare pointers with non-integral types");
}

template<typename T>
inline BINOP_HANDLER(ValueType<T>::RGt, PointerType) {
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

template<typename T>
inline BINOP_HANDLER(ValueType<T>::RArrayAccess, PointerType) {
    if constexpr(std::is_integral_v<T>) {
        return other.Deref();
    }
    else {
        throw std::runtime_error("Non-integral expression in array access");
    }
}