#include "Types.h"
#include "Log.h"
#include "SStream.h"

#include <functional>
#include <utility>

namespace epi {

std::string StructUnionType::ToString() const {
  cotyl::StringStream repr{};
  repr << BaseString();
  if (!name.empty()) {
    repr << ' ' << name;
  }
  repr << " {";

  for (const auto& field : fields) {
    repr << "\n  " << stringify(field.type) << ' ' << field.name;
    if (field.size) {
      repr << " : " << field.size;
    }
    repr << ';';
  }
  repr << "\n}";
  return repr.finalize();
}

u64 StructUnionType::Sizeof() const {
  // todo: different for union
  if (!IsComplete()) {
    CType::Sizeof();
    return 0;
  }
  u64 value = 0;
  for (const auto& field : fields) {
    u64 align = field.type->Alignof();

    // padding
    if (value & (align - 1)) {
      value += align - (value & (align - 1));
    }
    value += field.type->Sizeof();
  }
  return value;
}


bool StructUnionType::_EqualTypeImpl(const StructUnionType& other) const {
  if (fields.size() != other.fields.size()) {
    return false;
  }

  if (!IsComplete()) {
    return false;
  }

  for (size_t i = 0; i < fields.size(); i++) {
    const auto& this_field = fields[i];
    const auto& other_field = other.fields[i];
    if (this_field.name != other_field.name) {
      return false;
    }
    if (this_field.size != other_field.size) {
      return false;
    }
    if (!(*this_field.type).EqualType(*other_field.type)) {
      return false;
    }
  }

  return true;
}

std::string FunctionType::Arg::ToString() const {
  if (name.empty()) {
    return stringify(type);
  }
  return cotyl::FormatStr("%s %s", type, name);
}

std::string FunctionType::ToString() const {
  cotyl::StringStream repr{};
  std::string formatted = cotyl::FormatStr("(%s)(", contained ? stringify(contained) : "%%");
  repr << formatted;
  repr << cotyl::Join(", ", arg_types);
  if (variadic) {
    repr << ", ...";
  }
  repr << ')';
  return repr.finalize();
}

pType<> FunctionType::FunctionCall(const std::vector<pType<const CType>>& args) const {
  if (args.size() != arg_types.size()) {
    if (!variadic || args.size() < arg_types.size()) {
      throw std::runtime_error("Not enough arguments for function call");
    }
  }

  for (int i = 0; i < arg_types.size(); i++) {
    if (!arg_types[i].type->Cast(*args[i])) {
      throw std::runtime_error("Cannot cast argument type");
    }
  }
  return contained->Clone();
}

bool FunctionType::EqualTypeImpl(const FunctionType& other) const {
  if (!(*contained).EqualType(*other.contained)) {
    return false;
  }
  if (arg_types.size() != other.arg_types.size()) {
    return false;
  }

  for (size_t i = 0; i < arg_types.size(); i++) {
    if (!(arg_types[i].type->EqualType(*other.arg_types[i].type))) {
      return false;
    }
  }

  return true;
}

pType<> FunctionType::Clone() const {
  auto clone = MakeType<FunctionType>(contained, variadic, lvalue, qualifiers);
  for (const auto& arg : arg_types) {
    clone->AddArg(arg.name, arg.type);
  }
  return clone;
}

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
  same_type->lvalue = LValueNess::None;
  return same_type;
}

BINOP_HANDLER(PointerType::RAdd, ValueType<u8>) {
  auto same_type = Clone();
  same_type->ForgetConstInfo();
  same_type->lvalue = LValueNess::None;
  return same_type;
}

BINOP_HANDLER(PointerType::RAdd, ValueType<u16>) {
  auto same_type = Clone();
  same_type->ForgetConstInfo();
  same_type->lvalue = LValueNess::None;
  return same_type;
}

BINOP_HANDLER(PointerType::RAdd, ValueType<i16>) {
  auto same_type = Clone();
  same_type->ForgetConstInfo();
  same_type->lvalue = LValueNess::None;
  return same_type;
}

BINOP_HANDLER(PointerType::RAdd, ValueType<u32>) {
  auto same_type = Clone();
  same_type->ForgetConstInfo();
  same_type->lvalue = LValueNess::None;
  return same_type;
}

BINOP_HANDLER(PointerType::RAdd, ValueType<i32>) {
  auto same_type = Clone();
  same_type->ForgetConstInfo();
  same_type->lvalue = LValueNess::None;
  return same_type;
}

BINOP_HANDLER(PointerType::RAdd, ValueType<u64>) {
  auto same_type = Clone();
  same_type->ForgetConstInfo();
  same_type->lvalue = LValueNess::None;
  return same_type;
}

BINOP_HANDLER(PointerType::RAdd, ValueType<i64>) {
  auto same_type = Clone();
  same_type->ForgetConstInfo();
  same_type->lvalue = LValueNess::None;
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

BINOP_HANDLER(PointerType::RCommonType, ValueType<i8>) { return Clone(); }
BINOP_HANDLER(PointerType::RCommonType, ValueType<u8>) { return Clone(); }
BINOP_HANDLER(PointerType::RCommonType, ValueType<u16>) { return Clone(); }
BINOP_HANDLER(PointerType::RCommonType, ValueType<i16>) { return Clone(); }
BINOP_HANDLER(PointerType::RCommonType, ValueType<u32>) { return Clone(); }
BINOP_HANDLER(PointerType::RCommonType, ValueType<i32>) { return Clone(); }
BINOP_HANDLER(PointerType::RCommonType, ValueType<u64>) { return Clone(); }
BINOP_HANDLER(PointerType::RCommonType, ValueType<i64>) { return Clone(); }

pType<> CType::Cast(const CType& other) const {
  u32 flagdiff = other.qualifiers & ~other.qualifiers;
  if (flagdiff & Qualifier::Const) {
    Log::Warn("Casting drops 'const' qualifier");
  }
  if (flagdiff & Qualifier::Volatile) {
    Log::Warn("Casting drops 'volatile' qualifier");
  }
  // todo: restrict/thread local
  auto cast = DoCast(other);
  cast->lvalue = LValueNess::None;
  return cast;
}

pType<ValueType<i32>> CType::MakeBool(bool value) {
  // make bool with value
  return MakeType<ValueType<i32>>(value, LValueNess::None, 0);
}

pType<ValueType<i32>> CType::MakeBool() {
  // make bool without value
  return MakeType<ValueType<i32>>(LValueNess::None, 0);
}

pType<ValueType<i32>> CType::TruthinessAsCType() const {
  if (IsConstexpr()) {
    return MakeType<ValueType<i32>>(GetBoolValue() ? 1 : 0, LValueNess::None, 0);
  }
  return MakeBool();
}

pType<ValueType<i32>> CType::LogAnd(const CType& other) const {
  auto l = this->TruthinessAsCType();
  auto r = other.TruthinessAsCType();
  if (l->HasValue()) {
    if (!l->Get()) {
      return MakeBool(false);
    }
    else if (r->HasValue() && r->Get()) {
      return MakeBool(true);
    }
  }
  if (r->HasValue()) {
    if (!r->Get()) {
      return MakeBool(false);
    }
  }
  return MakeBool();
}

pType<ValueType<i32>> CType::LogOr(const CType& other) const {
  auto l = this->TruthinessAsCType();
  auto r = other.TruthinessAsCType();
  if (l->HasValue()) {
    if (l->Get()) {
      return MakeBool(true);
    }
    else if (r->HasValue() && !r->Get()) {
      return MakeBool(false);
    }
  }
  if (r->HasValue()) {
    if (r->Get()) {
      return MakeBool(true);
    }
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

pType<> CType::ArrayAccess(const CType& other) const {
  return Add(other)->Deref();
}

UNOP_HANDLER(CType::Incr) {
  if (!IsAssignable()) {
    throw std::runtime_error("Cannot increment non-assignable expression");
  }
  return Add(*ConstOne());
}

UNOP_HANDLER(CType::Decr) {
  if (!IsAssignable()) {
    throw std::runtime_error("Cannot decrement non-assignable expression");
  }
  return Sub(*ConstOne());
}

pType<> CType::Gt(const CType& other) const {
  return Le(other)->LogNot();  // !(<=)
}

pType<> CType::Le(const CType& other) const {
  auto lt = this->Lt(other);
  auto eq = this->Eq(other);
  return lt->LogOr(*eq);  // < || ==
}

pType<> CType::Ge(const CType& other) const {
  auto gt = this->Gt(other);
  auto eq = this->Eq(other);
  return gt->LogOr(*eq);  // > || ==
}

pType<> CType::Neq(const CType& other) const {
  auto eq = this->Eq(other);
  return eq->LogNot();   // !( == )
}

pType<ValueType<i8>> CType::ConstOne() {
  return MakeType<ValueType<i8>>(1, LValueNess::None);
}

pType<> StructUnionType::MemberAccess(const std::string& member) const {
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