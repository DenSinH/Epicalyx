#include "Types.h"


#include <functional>
#include <utility>

namespace epi {

UNOP_HANDLER(PointerType::Deref) {
  return contained->Clone();
}

UNOP_HANDLER(FunctionType::Deref) {
  // dereferencing a function pointer returns the function pointer itself
  return Clone();
}

BINOP_HANDLER(PointerType::RSub, PointerType) {
  if ((*this).EqualType(other)) {
    return MakeType<ValueType<u64>>(LValueNess::None, 0);
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


BINOP_HANDLER(PointerType::RArrayAccess, ValueType<i8>) {
  auto same_type = contained->Clone();
  same_type->ForgetConstInfo();
  same_type->SetNotLValue();
  return same_type;
}

BINOP_HANDLER(PointerType::RArrayAccess, ValueType<u8>) {
  auto same_type = contained->Clone();
  same_type->ForgetConstInfo();
  same_type->SetNotLValue();
  return same_type;
}

BINOP_HANDLER(PointerType::RArrayAccess, ValueType<u16>) {
  auto same_type = contained->Clone();
  same_type->ForgetConstInfo();
  same_type->SetNotLValue();
  return same_type;
}

BINOP_HANDLER(PointerType::RArrayAccess, ValueType<i16>) {
  auto same_type = contained->Clone();
  same_type->ForgetConstInfo();
  same_type->SetNotLValue();
  return same_type;
}

BINOP_HANDLER(PointerType::RArrayAccess, ValueType<u32>) {
  auto same_type = contained->Clone();
  same_type->ForgetConstInfo();
  same_type->SetNotLValue();
  return same_type;
}

BINOP_HANDLER(PointerType::RArrayAccess, ValueType<i32>) {
  auto same_type = contained->Clone();
  same_type->ForgetConstInfo();
  same_type->SetNotLValue();
  return same_type;
}

BINOP_HANDLER(PointerType::RArrayAccess, ValueType<u64>) {
  auto same_type = contained->Clone();
  same_type->ForgetConstInfo();
  same_type->SetNotLValue();
  return same_type;
}

BINOP_HANDLER(PointerType::RArrayAccess, ValueType<i64>) {
  auto same_type = contained->Clone();
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
  } else if (contained->EqualType(VoidType())) {
    // also allowed
    return MakeBool();
  } else if (other.contained->EqualType(VoidType())) {
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

pType<ValueType<i32>> CType::MakeBool(bool value) {
  // make bool with value
  return MakeType<ValueType<i32>>(value, LValueNess::None, 0);
}

pType<ValueType<i32>> CType::MakeBool() {
  // make bool without value
  return MakeType<ValueType<i32>>(LValueNess::None, 0);
}

pType<ValueType<i32>> CType::LogAnd(const CType& other) const {
  auto l = this->TruthinessAsCType();
  auto r = other.TruthinessAsCType();
  if (l->HasValue() && r->HasValue()) {
    return MakeBool(l->Get() && r->Get());
  }
  return MakeBool();
}

pType<ValueType<i32>> CType::LogOr(const CType& other) const {
  auto l = this->TruthinessAsCType();
  auto r = other.TruthinessAsCType();
  if (l->HasValue() && r->HasValue()) {
    return MakeBool(l->Get() || r->Get());
  }
  return MakeBool();
}

pType<ValueType<i32>> CType::LogNot() const {
  auto bool_val = TruthinessAsCType();
  if (bool_val->HasValue()) {
    return MakeBool(!bool_val->Get());
  }
  return MakeBool();
}

pType<> CType::LLogAnd(const CType& other) const { return LogAnd(other); }
pType<> CType::LLogOr(const CType& other) const { return LogAnd(other); }
pType<> CType::LLogNot() const { return LogNot(); }

UNOP_HANDLER(CType::Ref) {
  // only for lvalues
  // not an lvalue after
  if (lvalue == LValueNess::None) {
    throw std::runtime_error("Cannot get reference to non-lvalue expression");
  }
  return MakeType<PointerType>(Clone(), LValueNess::None);
}

UNOP_HANDLER(CType::Incr) {
  return Add(*ConstOne());
}

UNOP_HANDLER(CType::Decr) {
  return Sub(*ConstOne());
}

pType<> CType::Le(const CType& other) const {
  auto lt = this->Lt(other);
  auto eq = this->Eq(other);
  return lt->LogOr(other);  // < || ==
}

pType<> CType::Ge(const CType& other) const {
  auto gt = this->Gt(other);
  auto eq = this->Eq(other);
  return gt->LogOr(other);  // > || ==
}

pType<> CType::Neq(const CType& other) const {
  auto eq = this->Eq(other);
  return eq->LogNot();   // !( == )
}

pType<ValueType<i8>> CType::ConstOne() {
  return MakeType<ValueType<i8>>(1, LValueNess::None);
}

template<int type>
pType<> StructUnionType<type>::MemberAccess(const std::string& member) const {
  for (auto& field : fields) {
    if (field.name == member) {
      return field.type->Clone();
    }
  }
  throw std::runtime_error("No field named " + member + " in " + ToString());
}

#include "ValueTypeMethods.inl"

template struct ValueType<i8>;
template struct ValueType<u8>;
template struct ValueType<i16>;
template struct ValueType<u16>;
template struct ValueType<i32>;
template struct ValueType<u32>;
template struct ValueType<i64>;
template struct ValueType<u64>;
template struct ValueType<float>;
template struct ValueType<double>;

}