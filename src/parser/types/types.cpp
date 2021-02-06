#include "types.h"
#include <iostream>

BINOP_HANDLER(PointerType::RSub, PointerType) {
    if ((*this).EqualTypeImpl(other)) {
        return MAKE_TYPE(ValueType<u64>)(0);
    }
    throw std::runtime_error("Cannot subtract pointers of non-compatible types");
}

BINOP_HANDLER(PointerType::RAdd, ValueType<i8>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RAdd, ValueType<u8>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RAdd, ValueType<u16>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RAdd, ValueType<i16>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RAdd, ValueType<u32>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RAdd, ValueType<i32>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RAdd, ValueType<u64>) { return MAKE_TYPE(PointerType)(Contained, Flags); }
BINOP_HANDLER(PointerType::RAdd, ValueType<i64>) { return MAKE_TYPE(PointerType)(Contained, Flags); }

BINOP_HANDLER(PointerType::RLt, PointerType) {
    if ((*this).EqualTypeImpl(other)) {
        return MAKE_TYPE(ValueType<i32>)(0);
    }
    throw std::runtime_error("Cannot compare pointers of non-compatible types");
}

template<typename T>
BINOP_HANDLER(ValueType<T>::RLt, PointerType) {
    if constexpr(std::is_integral_v<T>) {
        return MAKE_TYPE(ValueType<i32>)(0);
    }
    throw std::runtime_error("Cannot compare pointers with non-integral types");
}

BINOP_HANDLER(PointerType::RLt, ValueType<i8>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::RLt, ValueType<u8>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::RLt, ValueType<u16>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::RLt, ValueType<i16>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::RLt, ValueType<u32>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::RLt, ValueType<i32>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::RLt, ValueType<u64>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::RLt, ValueType<i64>) { return MAKE_TYPE(ValueType<i32>)(0); }

BINOP_HANDLER(PointerType::RGt, PointerType) {
    if ((*this).EqualTypeImpl(other)) {
        return MAKE_TYPE(ValueType<i32>)(0);
    }
    throw std::runtime_error("Cannot compare pointers of non-compatible types");
}

template<typename T>
BINOP_HANDLER(ValueType<T>::RGt, PointerType) {
    if constexpr(std::is_integral_v<T>) {
        return MAKE_TYPE(ValueType<i32>)(0);
    }
    throw std::runtime_error("Cannot compare pointers with non-integral types");
}

BINOP_HANDLER(PointerType::RGt, ValueType<i8>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::RGt, ValueType<u8>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::RGt, ValueType<u16>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::RGt, ValueType<i16>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::RGt, ValueType<u32>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::RGt, ValueType<i32>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::RGt, ValueType<u64>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::RGt, ValueType<i64>) { return MAKE_TYPE(ValueType<i32>)(0); }

BINOP_HANDLER(PointerType::REq, PointerType) {
    if ((*this).EqualTypeImpl(other)) {
        return MAKE_TYPE(ValueType<i32>)(0);
    }
    else if (Contained->Base == BaseType::Void) {
        // also allowed
        return MAKE_TYPE(ValueType<i32>)(0);
    }
    else if (other.Contained->Base == BaseType::Void) {
        // also allowed
        return MAKE_TYPE(ValueType<i32>)(0);
    }
    throw std::runtime_error("Cannot compare pointers of non-compatible types");
}

template<typename T>
BINOP_HANDLER(ValueType<T>::REq, PointerType) {
    if constexpr(std::is_integral_v<T>) {
        return MAKE_TYPE(ValueType<i32>)(0);
    }
    throw std::runtime_error("Cannot compare pointers with non-integral types");
}

BINOP_HANDLER(PointerType::REq, ValueType<i8>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::REq, ValueType<u8>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::REq, ValueType<u16>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::REq, ValueType<i16>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::REq, ValueType<u32>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::REq, ValueType<i32>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::REq, ValueType<u64>) { return MAKE_TYPE(ValueType<i32>)(0); }
BINOP_HANDLER(PointerType::REq, ValueType<i64>) { return MAKE_TYPE(ValueType<i32>)(0); }

TYPE(ValueType<i32>) CType::LogAnd(const CTYPE& other) const {
    auto l = this->BoolVal();
    auto r = this->BoolVal();
    if (l->HasValue() && r->HasValue()) {
        return MAKE_TYPE(ValueType<i32>)(l->Get() && r->Get(), 0);
    }
    return MAKE_TYPE(ValueType<i32>)(0);
}

TYPE(ValueType<i32>) CType::LogOr(const CTYPE& other) const {
    auto l = this->BoolVal();
    auto r = this->BoolVal();
    if (l->HasValue() && r->HasValue()) {
        return MAKE_TYPE(ValueType<i32>)(l->Get() || r->Get(), 0);
    }
    return MAKE_TYPE(ValueType<i32>)(0);
}

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
BINOP_HANDLER(ValueType<T>::R ## handler, ValueType<type>) { \
    using common_t = std::common_type_t<T, type>; \
    return ValueTypeRBinOp<type>(other, functional); \
}

#define INTEGRAL_VALUE_TYPE_BINOP_HANDLER(handler, functional, type) \
template<typename T> \
BINOP_HANDLER(ValueType<T>::R ## handler, ValueType<type>) { \
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
BINOP_HANDLER(ValueType<T>::R ## handler, ValueType<type>) { \
    using common_t = std::common_type_t<T, type>; \
    return ValueTypeRBoolBinOp<type>(other, functional); \
}

template<typename T>
BINOP_HANDLER(ValueType<T>::RSub, PointerType) {
    return MAKE_TYPE(PointerType)(other.Contained, other.Flags);
}

template<typename T>
BINOP_HANDLER(ValueType<T>::RAdd, PointerType) {
    return MAKE_TYPE(PointerType)(other.Contained, other.Flags);
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

VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BOOL_BINOP_HANDLER, Lt, [](common_t l, common_t r) -> i8 { return l < r; });
VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BOOL_BINOP_HANDLER, Gt, [](common_t l, common_t r) -> i8 { return l < r; });
VALUE_TYPE_NUMERIC_TYPE_HANDLERS(VALUE_TYPE_BOOL_BINOP_HANDLER, Eq, [](common_t l, common_t r) -> i8 { return l == r; });