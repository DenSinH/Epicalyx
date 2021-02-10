#include "types.h"
#include <iostream>

UNOP_HANDLER(PointerType::Deref) {
    return Contained->Clone();
}

UNOP_HANDLER(FunctionType::Deref) {
    // dereferencing a function pointer returns the function pointer itself
    return Clone();
}

BINOP_HANDLER(PointerType::RSub, PointerType) {
    if ((*this).EqualType(other)) {
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
    if ((*this).EqualType(other)) {
        return MakeBool();
    }
    throw std::runtime_error("Cannot compare pointers of non-compatible types");
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
    if ((*this).EqualType(other)) {
        return MakeBool();
    }
    throw std::runtime_error("Cannot compare pointers of non-compatible types");
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
    if ((*this).EqualType(other)) {
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

template<CType::BaseType type>
CTYPE StructUnionType<type>::MemberAccess(const std::string& member) const {
    for (auto& field : Fields) {
        if (field.Name == member) {
            return field.Type->Clone();
        }
    }
    throw std::runtime_error("No field named " + member + " in " + ToString());
}
