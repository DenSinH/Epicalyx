#include "types.h"
#include <iostream>

BINOP_HANDLER(PointerType::RSub, PointerType) {
    if ((*this).EqualTypeImpl(other)) {
        return MAKE_TYPE(ValueType<u64>)(LValueNess::None, 0);
    }
    throw std::runtime_error("Cannot subtract pointers of non-compatible types");
}

BINOP_HANDLER(PointerType::RAdd, ValueType<i8>) {
    auto same_type = Clone();
    same_type->ForgetConstInfo();
    same_type->SetNotLValue();
    return same_type;
}

BINOP_HANDLER(PointerType::RAdd, ValueType<u8>) {
    auto same_type = Clone();
    same_type->ForgetConstInfo();
    same_type->SetNotLValue();
    return same_type;
}

BINOP_HANDLER(PointerType::RAdd, ValueType<u16>) {
    auto same_type = Clone();
    same_type->ForgetConstInfo();
    same_type->SetNotLValue();
    return same_type;
}

BINOP_HANDLER(PointerType::RAdd, ValueType<i16>) {
    auto same_type = Clone();
    same_type->ForgetConstInfo();
    same_type->SetNotLValue();
    return same_type;
}

BINOP_HANDLER(PointerType::RAdd, ValueType<u32>) {
    auto same_type = Clone();
    same_type->ForgetConstInfo();
    same_type->SetNotLValue();
    return same_type;
}

BINOP_HANDLER(PointerType::RAdd, ValueType<i32>) {
    auto same_type = Clone();
    same_type->ForgetConstInfo();
    same_type->SetNotLValue();
    return same_type;
}

BINOP_HANDLER(PointerType::RAdd, ValueType<u64>) {
    auto same_type = Clone();
    same_type->ForgetConstInfo();
    same_type->SetNotLValue();
    return same_type;
}

BINOP_HANDLER(PointerType::RAdd, ValueType<i64>) {
    auto same_type = Clone();
    same_type->ForgetConstInfo();
    same_type->SetNotLValue();
    return same_type;
}

BINOP_HANDLER(PointerType::RLt, PointerType) {
    if ((*this).EqualTypeImpl(other)) {
        return MakeBool();
    }
    throw std::runtime_error("Cannot compare pointers of non-compatible types");
}

template<typename T>
BINOP_HANDLER(ValueType<T>::RLt, PointerType) {
    if constexpr(std::is_integral_v<T>) {
        return MakeBool();
    }
    throw std::runtime_error("Cannot compare pointers with non-integral types");
}

BINOP_HANDLER(PointerType::RLt, ValueType<i8>) { return MakeBool(); }
BINOP_HANDLER(PointerType::RLt, ValueType<u8>) { return MakeBool(); }
BINOP_HANDLER(PointerType::RLt, ValueType<u16>) { return MakeBool(); }
BINOP_HANDLER(PointerType::RLt, ValueType<i16>) { return MakeBool(); }
BINOP_HANDLER(PointerType::RLt, ValueType<u32>) { return MakeBool(); }
BINOP_HANDLER(PointerType::RLt, ValueType<i32>) { return MakeBool(); }
BINOP_HANDLER(PointerType::RLt, ValueType<u64>) { return MakeBool(); }
BINOP_HANDLER(PointerType::RLt, ValueType<i64>) { return MakeBool(); }

BINOP_HANDLER(PointerType::RGt, PointerType) {
    if ((*this).EqualTypeImpl(other)) {
        return MakeBool();
    }
    throw std::runtime_error("Cannot compare pointers of non-compatible types");
}

template<typename T>
BINOP_HANDLER(ValueType<T>::RGt, PointerType) {
    if constexpr(std::is_integral_v<T>) {
        return MakeBool();
    }
    throw std::runtime_error("Cannot compare pointers with non-integral types");
}

BINOP_HANDLER(PointerType::RGt, ValueType<i8>) { return MakeBool(); }
BINOP_HANDLER(PointerType::RGt, ValueType<u8>) { return MakeBool(); }
BINOP_HANDLER(PointerType::RGt, ValueType<u16>) { return MakeBool(); }
BINOP_HANDLER(PointerType::RGt, ValueType<i16>) { return MakeBool(); }
BINOP_HANDLER(PointerType::RGt, ValueType<u32>) { return MakeBool(); }
BINOP_HANDLER(PointerType::RGt, ValueType<i32>) { return MakeBool(); }
BINOP_HANDLER(PointerType::RGt, ValueType<u64>) { return MakeBool(); }
BINOP_HANDLER(PointerType::RGt, ValueType<i64>) { return MakeBool(); }

BINOP_HANDLER(PointerType::REq, PointerType) {
    if ((*this).EqualTypeImpl(other)) {
        return MakeBool();
    }
    else if (Contained->Base == BaseType::Void) {
        // also allowed
        return MakeBool();
    }
    else if (other.Contained->Base == BaseType::Void) {
        // also allowed
        return MakeBool();
    }
    throw std::runtime_error("Cannot compare pointers of non-compatible types");
}

template<typename T>
BINOP_HANDLER(ValueType<T>::REq, PointerType) {
    if constexpr(std::is_integral_v<T>) {
        return MakeBool();
    }
    throw std::runtime_error("Cannot compare pointers with non-integral types");
}

BINOP_HANDLER(PointerType::REq, ValueType<i8>) { return MakeBool(); }
BINOP_HANDLER(PointerType::REq, ValueType<u8>) { return MakeBool(); }
BINOP_HANDLER(PointerType::REq, ValueType<u16>) { return MakeBool(); }
BINOP_HANDLER(PointerType::REq, ValueType<i16>) { return MakeBool(); }
BINOP_HANDLER(PointerType::REq, ValueType<u32>) { return MakeBool(); }
BINOP_HANDLER(PointerType::REq, ValueType<i32>) { return MakeBool(); }
BINOP_HANDLER(PointerType::REq, ValueType<u64>) { return MakeBool(); }
BINOP_HANDLER(PointerType::REq, ValueType<i64>) { return MakeBool(); }

TYPE(ValueType<i32>) CType::MakeBool(bool value) {
    // make bool with value
    return MAKE_TYPE(ValueType<i32>)(value, LValueNess::None, 0);
}

TYPE(ValueType<i32>) CType::MakeBool() {
    // make bool without value
    return MAKE_TYPE(ValueType<i32>)(LValueNess::None, 0);
}

TYPE(ValueType<i32>) CType::LogAnd(const CType& other) const {
    auto l = this->TruthinessAsCType();
    auto r = other.TruthinessAsCType();
    if (l->HasValue() && r->HasValue()) {
        return MakeBool(l->Get() && r->Get());
    }
    return MakeBool();
}

TYPE(ValueType<i32>) CType::LogOr(const CType& other) const {
    auto l = this->TruthinessAsCType();
    auto r = other.TruthinessAsCType();
    if (l->HasValue() && r->HasValue()) {
        return MakeBool(l->Get() || r->Get());
    }
    return MakeBool();
}

TYPE(ValueType<i32>) CType::LogNot() const {
    auto bool_val = TruthinessAsCType();
    if (bool_val->HasValue()) {
        return MakeBool(!bool_val->Get());
    }
    return MakeBool();
}

UNOP_HANDLER(CType::Ref) {
    // only for lvalues
    // not an lvalue after
    if (LValue == LValueNess::None) {
        throw std::runtime_error("Cannot get reference to non-lvalue expression");
    }
    return MAKE_TYPE(PointerType)(Clone(), LValueNess::None);
}

CTYPE CType::Le(const CType& other) const {
    auto lt = this->Lt(other);
    auto eq = this->Eq(other);
    return lt->LogOr(other);  // < || ==
}

CTYPE CType::Ge(const CType& other) const {
    auto gt = this->Gt(other);
    auto eq = this->Eq(other);
    return gt->LogOr(other);  // > || ==
}

CTYPE CType::Neq(const CType& other) const {
    auto eq = this->Eq(other);
    return eq->LogNot();   // !( == )
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
    auto same_type = other.Clone();
    same_type->ForgetConstInfo();
    same_type->SetNotLValue();
    return same_type;
}

template<typename T>
BINOP_HANDLER(ValueType<T>::RAdd, PointerType) {
    auto same_type = other.Clone();
    same_type->ForgetConstInfo();
    same_type->SetNotLValue();
    return same_type;
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