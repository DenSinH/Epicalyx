#include "Types.h"

#include <stddef.h>                    // for size_t
#include <algorithm>                   // for max, copy
#include <concepts>                    // for integral
#include <functional>                  // for divides, modulus, bit_and, bit_or
#include <iterator>                    // for back_insert_iterator, back_ins...
#include <tuple>                       // for tuple
#include <type_traits>                 // for remove_reference, common_type_t
#include <utility>                     // for move

#include "AnyType.h"                   // for AnyType, any_type_t
#include "CustomAssert.h"              // for Assert
#include "Decltype.h"                  // for decltype_t
#include "Format.h"                    // for FormatStr, FormatExceptStr, Join
#include "Log.h"                       // for Warn
#include "SStream.h"                   // for StringStream
#include "Stringify.h"                 // for stringify
#include "TypeUtils.h"                 // for type_string_v
#include "Variant.h"                   // for Variant
#include "Vector.h"                    // for vector, vec_iterator, operator-


namespace epi::type {

template<typename T1, typename T2>
using common_type_t = std::common_type_t<T1, T2>;

using ::epi::stringify;

[[noreturn]] static void InvalidOperands(const BaseType* ths, const std::string& op, const AnyType& other) {
  throw cotyl::FormatExceptStr<TypeError>(
    "Invalid operands for %s: %s and %s",
    op, *ths, other
  );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * VOID TYPES
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

AnyType VoidType::CommonTypeImpl(const AnyType& other) const {
  throw TypeError("Cannot determine common type of incomplete type");
}

u64 VoidType::Sizeof() const { 
  throw TypeError("Cannot determine size of incomplete type");
}

std::string VoidType::ToString() const { 
  return "void"; 
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * VALUE TYPES
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
template<template<typename S> class handler, typename T, typename R>
AnyType BinopHelper(const ValueType<T>& one,  const ValueType<R>& other) {
  using common_t = common_type_t<T, R>;
  if (one.value.has_value() && other.value.has_value()) {
    handler<common_t> h;
    using return_t = std::conditional_t<std::is_same_v<decltype_t(h(one.value.value(), other.value.value())), bool>, i32, decltype_t(h(one.value.value(), other.value.value()))>;
    if constexpr(std::is_same_v<handler<common_t>, std::divides<common_t>> || std::is_same_v<handler<common_t>, std::modulus<common_t>>) {
      if (other.value.value() == 0) {
        // todo: fix this (lazy eval in constexpr, see 0160-cpp_if.c)
        return ValueType<return_t>(LValue::None);
      }
    }
    auto return_val = h(one.value.value(), other.value.value());
    return ValueType<return_t>(return_val, LValue::None);
  }
  return ValueType<common_t>(LValue::None);
}

template<template<typename S> class handler, typename T>
AnyType NonAdditiveBinopHelper(const ValueType<T>& one, const char* op_str, const AnyType& other) {
  auto result = other.visit<AnyType>(
    [&]<typename R>(const ValueType<R>& other) {
      return BinopHelper<handler>(one, other);
    },
    [&](const auto& other) -> AnyType {
      InvalidOperands(&one, op_str, other);
    }
  );
  result->lvalue = LValue::None;
  return std::move(result);
}


template<template<typename S> class handler, typename T>
BoolType BooleanBinopHelper(const ValueType<T>& one, const char* op_str, const AnyType& other) {
  auto result = other.visit<BoolType>(
    [](const PointerType&) { return BoolType(LValue::None); },
    [](const FunctionType&) { return BoolType(LValue::None); },
    [](const ArrayType&) { return BoolType(LValue::None); },
    [&]<typename R>(const ValueType<R>& other) {
      using common_t = common_type_t<T, R>;
      if (one.value.has_value() && other.value.has_value()) {
        handler<common_t> h;
        bool return_val = h(one.value.value(), other.value.value());
        return BoolType(return_val ? 1 : 0, LValue::None);
      }
      return BoolType(LValue::None);
    },
    [&](const auto& other) -> BoolType {
      InvalidOperands(&one, op_str, other);
    }
  );
  result.lvalue = LValue::None;
  return std::move(result);
}

template<template<typename S> class handler, typename T>
AnyType IntegralBinopHelper(const ValueType<T>& one, const char* op_str, const AnyType& other) {
  if constexpr(!std::is_integral_v<T>) {
    InvalidOperands(&one, op_str, other);
  }
  else {
    auto result = other.visit<AnyType>(
      [&]<std::integral R>(const ValueType<R>& other) {
        return BinopHelper<handler>(one, other);
      },
      [&](const auto& other) -> AnyType {
        InvalidOperands(&one, op_str, other);
      }
    );
    result->lvalue = LValue::None;
    return std::move(result);
  }
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::Add(const AnyType& other) const {
  auto result = other.visit<AnyType>(
    [&](const PointerType& pointer) -> AnyType { 
      if (pointer.Stride() == 0) {
        // can't add to pointer to incomplete type
        InvalidOperands(this, "+", other);
      }
      PointerType result = pointer;
      result.ForgetConstInfo();  // forget constant info
      return result;
    },
    [&](const ArrayType& pointer) -> AnyType { 
      if (pointer.Stride() == 0) {
        // can't add to pointer to incomplete type
        InvalidOperands(this, "+", other);
      }
      PointerType result = pointer;
      result.ForgetConstInfo();  // forget constant info
      return result;
    },
    [&]<typename R>(const ValueType<R>& other) -> AnyType {
      return BinopHelper<std::plus>(*this, other);
    },
    [&](const auto& other) -> AnyType {
      InvalidOperands(this, "+", other);
    }
  );
  result->lvalue = LValue::None;
  return std::move(result);
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::Sub(const AnyType& other) const {
  return NonAdditiveBinopHelper<std::minus>(*this, "-", other);
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::Mul(const AnyType& other) const {
  return NonAdditiveBinopHelper<std::multiplies>(*this, "*", other);
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::Div(const AnyType& other) const {
  return NonAdditiveBinopHelper<std::divides>(*this, "/", other);
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::Mod(const AnyType& other) const {
  return IntegralBinopHelper<std::modulus>(*this, "%", other);
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::Xor(const AnyType& other) const {
  return IntegralBinopHelper<std::bit_xor>(*this, "%", other);
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::BinAnd(const AnyType& other) const {
  return IntegralBinopHelper<std::bit_and>(*this, "&", other);
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::BinOr(const AnyType& other) const {
  return IntegralBinopHelper<std::bit_or>(*this, "|", other);
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
BoolType ValueType<T>::Lt(const AnyType& other) const {
  return BooleanBinopHelper<std::less>(*this, "<", other);
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
BoolType ValueType<T>::Eq(const AnyType& other) const {
  return BooleanBinopHelper<std::equal_to>(*this, "==", other);
}

template<typename T> struct lshift { T operator()(const T& l, const T& r) const { return l << r; }};
template<typename T> struct rshift { T operator()(const T& l, const T& r) const { return l >> r; }};

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::LShift(const AnyType& other) const {
  return IntegralBinopHelper<lshift>(*this, "<<", other);
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::RShift(const AnyType& other) const {
  return IntegralBinopHelper<rshift>(*this, ">>", other);
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::Pos() const {
  auto result = *this;
  result.lvalue = LValue::None;
  return result;
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::Neg() const {
  if (value.has_value()) {
    return ValueType<T>{(T)-value.value(), LValue::None, 0};
  }
  return ValueType<T>{LValue::None, 0};
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::BinNot() const {
  if constexpr(!std::is_integral_v<T>) {
    throw TypeError("Binary operation on non-integral type");
  }
  else {
    if (value.has_value()) {
      return ValueType<T>{(T)~value.value(), LValue::None, 0};
    }
    return ValueType<T>{LValue::None, 0};
  }
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
BoolType ValueType<T>::Truthiness() const {
  if (value.has_value()) {
    return BoolType{value.value() ? 1 : 0, LValue::None};
  }
  return BoolType{LValue::None};
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::CommonTypeImpl(const AnyType& other) const {
    auto result = other.visit<AnyType>(
    [&](const PointerType& pointer) -> AnyType {
      return pointer; 
    },
    []<typename R>(const ValueType<R>& other) -> AnyType {
      return ValueType<common_type_t<T, R>>{LValue::None};
    },
    [&](const auto& other) -> AnyType {
      InvalidOperands(this, "operation", other);
    }
  );
  result->lvalue = LValue::None;
  return std::move(result);
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
std::string ValueType<T>::ToString() const {
  if (!HasValue()) { return type_string_v<T>; }
  return cotyl::FormatStr("%s:%s", type_string_v<T>, Get());
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * POINTER TYPES
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
AnyType PointerType::Add(const AnyType& other) const {
  return other.visit<AnyType>(
    [&](const PointerType& other) -> AnyType { 
      InvalidOperands(this, "+", other);
    },
    [&](const ArrayType& other) -> AnyType { 
      InvalidOperands(this, "+", other);
    },
    [&](const auto& other) {
      return other.Add(*this);
    }
  );
}

AnyType ArrayType::Add(const AnyType& other) const {
  return other.visit<AnyType>(
    [&](const PointerType& other) -> AnyType { 
      InvalidOperands(this, "+", other);
    },
    [&](const ArrayType& other) -> AnyType { 
      InvalidOperands(this, "+", other);
    },
    [&](const auto& other) {
      return other.Add(*this);
    }
  );
}

AnyType DataPointerType::Sub(const AnyType& other) const {
  return other.visit<AnyType>(
    [&](const PointerType& other) -> AnyType {
      if (contained->TypeEquals(*other.contained)) {
        return ValueType<i64>{LValue::None, 0};
      }
      InvalidOperands(this, "-", other);
    },
    [&](const ArrayType& other) -> AnyType {
      if (contained->TypeEquals(*other.contained)) {
        return ValueType<i64>{LValue::None, 0};
      }
      InvalidOperands(this, "-", other);
    },
    [&]<std::integral T>(const ValueType<T>& other) {
      return Add(other.Neg());
    },
    [&](const auto& other) -> AnyType {
      InvalidOperands(this, "-", other);
    }
  );
}

BoolType DataPointerType::Lt(const AnyType& other) const {
  return other.visit<BoolType>(
    [&](const PointerType& other) {
      if (!contained->TypeEquals(*other.contained)) {
        Log::Warn(cotyl::FormatStr("Comparing pointers to different types: %s and %s", *contained, *other.contained).c_str());
      }
      return BoolType(LValue::None);
    },
    [&](const ArrayType& other) {
      if (!contained->TypeEquals(*other.contained)) {
        Log::Warn(cotyl::FormatStr("Comparing pointers to different types: %s and %s", *contained, *other.contained).c_str());
      }
      return BoolType(LValue::None);
    },
    []<std::integral T>(const ValueType<T>& other) {
      return BoolType(LValue::None);
    },
    [&](const auto&) -> BoolType { InvalidOperands(this, "<", other); }
  );
}

BoolType DataPointerType::Eq(const AnyType& other) const {
  return other.visit<BoolType>(
    [&](const PointerType& other) {
      if (contained->holds_alternative<VoidType>() || other.contained->holds_alternative<VoidType>()) {
        // allowed
        return BoolType(LValue::None);
      }
      if (!contained->TypeEquals(*other.contained)) {
        Log::Warn(cotyl::FormatStr("Comparing pointers to different types: %s and %s", *contained, *other.contained).c_str());
      }
      return BoolType(LValue::None);
    },
    [&](const ArrayType& other) {
      if (contained->holds_alternative<VoidType>() || other.contained->holds_alternative<VoidType>()) {
        // allowed
        return BoolType(LValue::None);
      }
      if (!contained->TypeEquals(*other.contained)) {
        Log::Warn(cotyl::FormatStr("Comparing pointers to different types: %s and %s", *contained, *other.contained).c_str());
      }
      return BoolType(LValue::None);
    },
    []<std::integral T>(const ValueType<T>& other) {
      return BoolType(LValue::None);
    },
    [&](const auto&) -> BoolType { InvalidOperands(this, "<", other); }
  );
}

AnyType DataPointerType::Deref() const {
  return *contained;
}

AnyType PointerType::FunctionCall(const cotyl::vector<AnyType>& args) const {
  return contained->visit<AnyType>(
    [&](const FunctionType& func) {
      return func.FunctionCall(args);
    }, 
    [](const auto& contained) -> AnyType {
      throw cotyl::FormatExceptStr<TypeError>("Invalid operand for call: (%s)*", contained);
    }
  );
}

BoolType DataPointerType::Truthiness() const {
  return BoolType{LValue::None};
}

AnyType PointerType::CommonTypeImpl(const AnyType& other) const {
  auto result = other.visit<AnyType>(
    [&]<std::integral T>(const ValueType<T>& other) {
      return *this;
    },
    [&](const PointerType& other) {
      // common type of void* and other pointer is other pointer
      if (contained->holds_alternative<VoidType>()) {
        return other;
      }
      if (other.contained->holds_alternative<VoidType>()) {
        return *this;
      }
      InvalidOperands(this, "operation", other);
    },
    [&](const auto& other) -> AnyType {
      InvalidOperands(this, "operation", other);
    }
  );
  result->ForgetConstInfo();
  return std::move(result);
}

AnyType ArrayType::CommonTypeImpl(const AnyType& other) const {
  return other.visit<AnyType>(
    [&]<std::integral T>(const ValueType<T>& other) {
      PointerType result = *this;
      result.ForgetConstInfo();
      return std::move(result);
    },
    [&](const auto& other) -> AnyType {
      InvalidOperands(this, "operation", other);
    }
  );
}

u64 PointerType::Sizeof() const {
  return sizeof(u64);
}

u64 ArrayType::Sizeof() const {
  return size * (*contained)->Sizeof();
}

u32 ArrayType::Alignof() const {
  return (*contained)->Alignof();
}

u64 DataPointerType::Stride() const {
  // stride may be 0 for incomplete types
  return contained->visit<u64>(
    [](const VoidType&) -> u64 { return 0; },
    [](const StructType& strct) -> u64 {
      if (!strct.Complete()) return 0;
      return strct.Sizeof();
    },
    [](const UnionType& strct) -> u64 {
      if (!strct.Complete()) return 0;
      return strct.Sizeof();
    },
    [](const auto& type) -> u64 { return type.Sizeof(); }
  );
}

std::string PointerType::ToString() const {
  return cotyl::FormatStr("(%s)*", contained);
}

std::string ArrayType::ToString() const {
  if (!size) {
    return cotyl::FormatStr("(%s)[]", contained);
  }
  else {
    return cotyl::FormatStr("(%s)[%s]", contained, size);
  }
}

void PointerType::ForgetConstInfo() const {
  if (contained) (*contained)->ForgetConstInfo();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * FUNCTION TYPES
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void FunctionType::AddArg(cotyl::CString&& name, nested_type_t&& arg) {
  // constant info is nonsense for arguments
  (*arg)->ForgetConstInfo();
  arg_types.emplace_back(std::move(name), std::move(arg));
}

AnyType FunctionType::Deref() const { 
  // dereferencing a function pointer returns the function pointer itself
  return *this; 
}

bool FunctionType::TypeEqualImpl(const FunctionType& other) const {
  if (!contained->TypeEquals(*other.contained)) {
    return false;
  }
  if (arg_types.size() != other.arg_types.size()) {
    return false;
  }

  for (size_t i = 0; i < arg_types.size(); i++) {
    if (!arg_types[i].type->TypeEquals(*other.arg_types[i].type)) {
      return false;
    }
  }

  return true;
}

AnyType FunctionType::FunctionCall(const cotyl::vector<AnyType>& args) const {
  if (args.size() != arg_types.size()) {
    if (!variadic || args.size() < arg_types.size()) {
      throw TypeError("Not enough arguments for function call");
    }
  }

  for (int i = 0; i < arg_types.size(); i++) {
    // try to cast
    (*arg_types[i].type).Cast(args[i], false);
  }
  return *contained;
}

BoolType FunctionType::Truthiness() const {
  // symbol always has truthiness
  return BoolType(1, LValue::None);
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

AnyType FunctionType::CommonTypeImpl(const AnyType& other) const {
  return other.visit<AnyType>(
    [&]<std::integral T>(const ValueType<T>& other) {
      FunctionType result = *this;
      result.ForgetConstInfo();
      return std::move(result);
    },
    [&](const auto& other) -> AnyType {
      InvalidOperands(this, "operation", other);
    }
  );
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * STRUCT TYPES
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StructField::StructField(cotyl::CString&& name, size_t size, nested_type_t&& contained) :
    name{std::move(name)}, data{std::move(contained), size} { }

StructField::StructField(cotyl::CString&& name, nested_type_t&& contained) :
    StructField{std::move(name), 0, std::move(contained)} { }

StructUnionType::StructUnionType(
    cotyl::CString&& name,
    cotyl::vector<StructField>&& fields,
    LValue lvalue, 
    u8 flags
  ) : BaseType{lvalue, flags}, 
      name{std::move(name)}, 
      fields{std::move(fields)} {
  ComputeAlign();
}

void StructUnionType::ComputeAlign() {
  for (const auto& field : this->fields) {
    align = std::max(align, (*field.data.type)->Alignof());
  }
}

void StructUnionType::MergeFields(StructUnionType&& other) {
  cotyl::Assert(!Complete());
  // offsets are already computed
  fields = std::move(other.fields);
  align = other.align;
}

void StructUnionType::MergeFields(const StructUnionType& other) {
  cotyl::Assert(!Complete());
  // offsets are already computed
  std::copy(other.fields.begin(), other.fields.end(), std::back_inserter(fields));
  // use precomputed align
  align = other.align;
}

void StructUnionType::ForgetConstInfo() const {
  for (auto& field : fields) {
    (*field.data.type)->ForgetConstInfo();
  }
}

void StructType::ComputeOffsets() {
  u64 offset = 0;
  for (auto& field : fields) {
    // field.offset = current offset aligned to alignment
    const auto align_mask = (*field.data.type)->Alignof() - 1;
    offset = (offset + align_mask) & ~(align_mask);
    field.data.offset = offset;

    // add field size to current offset
    offset += (*field.data.type)->Sizeof();
  }
}

std::string StructUnionType::BodyString() const {
  cotyl::StringStream repr{};
  if (!name.empty()) {
    repr << name;
  }
  repr << "{";

  for (const auto& field : fields) {
    repr << "\n  " << stringify(field.data.type) << ' ' << field.name;
    if (field.data.size) {
      repr << " : " << field.data.size;
    }
    repr << ';';
  }
  repr << "\n}";
  return repr.finalize();
}

std::string StructType::ToString() const {
  return "struct " + BodyString();
}

std::string UnionType::ToString() const {
  return "union " + BodyString();
}

u64 StructType::Sizeof() const {
  if (fields.empty()) {
    throw TypeError("Cannot get size of incomplete struct definition");
  }
  
  // compute size by adding last field's size to it's offset
  // then round up to alignment
  const auto raw_size = fields.back().data.offset + (*fields.back().data.type)->Sizeof();
  const auto align_mask = align - 1;
  return (raw_size + align_mask) & ~align_mask;
}

u64 UnionType::Sizeof() const {
  // union size is max size of fields
  if (fields.empty()) {
    throw TypeError("Cannot get size of incomplete union definition");
  }
  u64 size = 0;
  for (const auto& field : fields) {
    size = std::max(size, (*field.data.type)->Sizeof());
  }
  return size;
}

StructFieldData StructUnionType::HasMember(const cotyl::CString& member) const {
  for (const auto& field : fields) {
    if (field.name == member) {
      // direct member
      return field.data;
    }
  }
  for (const auto& field : fields) {
    if (field.name.empty()) {
      return field.data.type->visit<StructFieldData>(
        [&](const StructType& strct) {
          auto nested = strct.HasMember(member);
          nested.offset += field.data.offset;
          return nested;
        },
        [&](const UnionType& strct) {
          auto nested = strct.HasMember(member);
          nested.offset += field.data.offset;
          return nested;
        },
        [](const auto&) -> StructFieldData { 
          return {nullptr}; 
        }
      );
    }
  }
  return {nullptr};
}

AnyType StructUnionType::MemberAccess(const cotyl::CString& member) const {
  auto mem = HasMember(member);
  if (!mem.type) {
    throw cotyl::FormatExceptStr<TypeError>("No field named %s in %s", member.c_str(), *this);
  }
  return *mem.type;
}

u64 StructUnionType::MemberOffset(const cotyl::CString& member) const {
  auto mem = HasMember(member);
  if (!mem.type) {
    throw cotyl::FormatExceptStr<TypeError>("No field named %s in %s", member.c_str(), *this);
  }
  return mem.offset;
}

AnyType StructUnionType::CommonTypeImpl(const AnyType& other) const {
  // TypeEqual is already checked
  InvalidOperands(this, "operation", other);
}

bool StructUnionType::TypeEqualImpl(const StructUnionType& other) const {
  if (!(name == other.name)) {
    return false;
  }

  // incomplete type
  if (fields.empty() || other.fields.empty()) {
    return true;
  }

  // not the same fields
  if (fields.size() != other.fields.size()) {
    return false;
  }

  for (size_t i = 0; i < fields.size(); i++) {
    const auto& this_field = fields[i];
    const auto& other_field = other.fields[i];
    if (this_field.name != other_field.name) {
      return false;
    }
    if (this_field.data.size != other_field.data.size) {
      return false;
    }
    if ((*this_field.data.type)->qualifiers != (*other_field.data.type)->qualifiers) {
      return false;
    }
    if (!(*this_field.data.type).TypeEquals(*other_field.data.type)) {
      return false;
    }
  }

  return true;
}

// force instantiation of all directives
template<typename...>
struct TypeInstantiator;

template<typename P, typename... Ts>
struct TypeInstantiator<cotyl::Variant<P, Ts...>> {
  std::tuple<Ts...> instantiated;
};

template struct TypeInstantiator<detail::any_type_t>;

}