#include "Types.h"
#include "AnyType.h"
#include "TypeTraits.h"
#include "TypeUtils.h"
#include "Stringify.h"
#include "Log.h"
#include "SStream.h"
#include "Decltype.h"
#include "Exceptions.h"

#include <functional>
#include <utility>
#include <tuple>


namespace epi::type {
  
template<typename T1, typename T2>
using common_type_t = std::common_type_t<T1, T2>;

using ::epi::stringify;

[[noreturn]] static void InvalidOperands(const BaseType* ths, const std::string& op, const AnyType& other) {
  throw cotyl::FormatExceptStr(
    "Invalid operands for %s: %s and %s",
    op, *ths, other
  );
}

AnyType MakeBool(BaseType::LValueNess lvalue, u8 flags) {
  return ValueType<i32>(lvalue, flags);
}

AnyType MakeBool(bool value, BaseType::LValueNess lvalue, u8 flags) {
  return ValueType<i32>(value ? 1 : 0, lvalue, flags);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * VOID TYPES
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

AnyType VoidType::CommonTypeImpl(const AnyType& other) const {
  throw std::runtime_error("Cannot determine common type of incomplete type");
}

u64 VoidType::Sizeof() const { 
  throw std::runtime_error("Cannot determine size of incomplete type");
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
    auto return_val = h(one.value.value(), other.value.value());
    using return_t = std::conditional_t<std::is_same_v<decltype_t(return_val), bool>, i32, decltype_t(return_val)>;
    return ValueType<return_t>(return_val, BaseType::LValueNess::None);
  }
  return ValueType<common_t>(BaseType::LValueNess::None);
}

template<template<typename S> class handler, typename T>
AnyType NonAdditiveBinopHelper(const ValueType<T>& one, const char* op_str, const AnyType& other) {
  auto result = other.visit<AnyType>(
    [&](const auto& other) -> AnyType {
      using other_t = decltype_t(other);
      if constexpr(cotyl::is_instantiation_of_v<ValueType, other_t>)
        return BinopHelper<handler>(one, other);
      else
        InvalidOperands(&one, op_str, other);
    }
  );
  result->lvalue = BaseType::LValueNess::None;
  return std::move(result);
}

template<template<typename S> class handler, typename T>
AnyType IntegralBinopHelper(const ValueType<T>& one, const char* op_str, const AnyType& other) {
  if constexpr(!std::is_integral_v<T>) {
    InvalidOperands(&one, op_str, other);
  }
  else {
    auto result = other.visit<AnyType>(
      [&](const auto& other) -> AnyType {
        using other_t = decltype_t(other);
        if constexpr(cotyl::is_instantiation_of_v<ValueType, other_t>)
          if constexpr(std::is_integral_v<typename other_t::type_t>)
            return BinopHelper<handler>(one, other);
          else 
            InvalidOperands(&one, op_str, other);
        else
          InvalidOperands(&one, op_str, other);
      }
    );
    result->lvalue = BaseType::LValueNess::None;
    return std::move(result);
  }
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::Add(const AnyType& other) const {
  auto result = other.visit<AnyType>(
    [](const PointerType& pointer) -> AnyType { 
      PointerType result = pointer;
      result.size = 0;           // lose array status
      result.ForgetConstInfo();  // forget constant info
      return result;
    },
    [&](const auto& other) -> AnyType {
        using other_t = decltype_t(other);
      if constexpr(cotyl::is_instantiation_of_v<ValueType, other_t>)
        return BinopHelper<std::plus>(*this, other);
      else
        InvalidOperands(this, "+", other);
    }
  );
  result->lvalue = BaseType::LValueNess::None;
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
AnyType ValueType<T>::Lt(const AnyType& other) const {
  return NonAdditiveBinopHelper<std::less>(*this, "<", other);
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::Eq(const AnyType& other) const {
  return NonAdditiveBinopHelper<std::equal_to>(*this, "==", other);
}

template<typename T> struct lshift { T operator()(const T& l, const T& r) const { return l < r; }};
template<typename T> struct rshift { T operator()(const T& l, const T& r) const { return l == r; }};

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
  result.lvalue = LValueNess::None;
  return result;
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::Neg() const {
  if (value.has_value()) {
    return ValueType<T>{(T)-value.value(), LValueNess::None, 0};
  }
  return ValueType<T>{LValueNess::None, 0};
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::BinNot() const {
  if constexpr(!std::is_integral_v<T>) {
    throw std::runtime_error("Binary operation on non-integral type");
  }
  else {
    if (value.has_value()) {
      return ValueType<T>{(T)~value.value(), LValueNess::None, 0};
    }
    return ValueType<T>{LValueNess::None, 0};
  }
}

template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
AnyType ValueType<T>::CommonTypeImpl(const AnyType& other) const {
    auto result = other.visit<AnyType>(
    [](const PointerType& pointer) -> AnyType { return pointer; },
    [&](const auto& other) -> AnyType {
      using other_t = decltype_t(other);
      if constexpr(cotyl::is_instantiation_of_v<ValueType, other_t>)
        return ValueType<common_type_t<T, typename other_t::type_t>>{LValueNess::None};
      else
        InvalidOperands(this, "operation", other);
    }
  );
  result->lvalue = BaseType::LValueNess::None;
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
    [&](const auto& other) {
      return other.Add(*this);
    }
  );
}

AnyType PointerType::Sub(const AnyType& other) const {
  return other.visit<AnyType>(
    [&](const PointerType& other) -> AnyType {
      if (contained->TypeEquals(*other.contained)) {
        return ValueType<u64>{LValueNess::None, 0};
      }
      InvalidOperands(this, "-", other);
    },
    [&](const auto& other) -> AnyType {
      InvalidOperands(this, "-", other);
    }
  );
}

AnyType PointerType::Lt(const AnyType& other) const {
  return other.visit<AnyType>(
    [&](const PointerType& other) {
      if (!contained->TypeEquals(*other.contained)) {
        InvalidOperands(this, "<", other); 
      }
      return MakeBool(LValueNess::None);
    },
    [&](const AnyValueType& other) {
      return MakeBool(LValueNess::None);
    },
    [&](const auto&) -> AnyType { InvalidOperands(this, "<", other); }
  );
}

AnyType PointerType::Eq(const AnyType& other) const {
  return other.visit<AnyType>(
    [&](const PointerType& other) {
      if (contained->holds_alternative<VoidType>()) {
        // allowed
        return MakeBool(LValueNess::None);
      }
      if (!contained->TypeEquals(*other.contained)) {
        InvalidOperands(this, "<", other); 
      }
      return MakeBool(LValueNess::None);
    },
    [&](const AnyValueType& other) {
      return MakeBool(LValueNess::None);
    },
    [&](const auto&) -> AnyType { InvalidOperands(this, "<", other); }
  );
}

AnyType PointerType::Deref() const {
  return *contained;
}

AnyType PointerType::CommonTypeImpl(const AnyType& other) const {
  return other.visit<AnyType>(
    [&](const auto& other) -> AnyType {
      using other_t = decltype_t(other);
      if constexpr(cotyl::is_instantiation_of_v<ValueType, other_t>) {
        if constexpr(std::is_integral_v<typename other_t::type_t>) {
          return *this;
        }
      }
      InvalidOperands(this, "operation", other);
    }
  );
}

u64 PointerType::Sizeof() const {
  if (size == 0) return sizeof(u64);
  return size * sizeof(u64);
}

std::string PointerType::ToString() const {
  if (size == 0) {
    return cotyl::FormatStr("(%s)*", contained);
  }
  else {
    return cotyl::FormatStr("(%s)[%s]", contained, size);
  }
}

u64 PointerType::Stride() const {
  return (*contained)->Sizeof();
}

void PointerType::ForgetConstInfo() const {
  if (contained) (*contained)->ForgetConstInfo();
}

AnyType PointerType::ToAny() {
  return *this;
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
      throw std::runtime_error("Not enough arguments for function call");
    }
  }

  for (int i = 0; i < arg_types.size(); i++) {
    // try to cast
    (*arg_types[i].type).Cast(args[i]);
  }
  return *contained;
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
    [&](const auto& other) -> AnyType {
      using other_t = decltype_t(other);
      if constexpr(cotyl::is_instantiation_of_v<ValueType, other_t>) {
        if constexpr(std::is_integral_v<typename other_t::type_t>) {
          return *this;
        }
      }
      InvalidOperands(this, "operation", other);
    }
  );
}

AnyType FunctionType::ToAny() {
  return *this;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 * STRUCT TYPES
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

StructField::StructField(cotyl::CString&& name, size_t size, nested_type_t&& contained) :
    name{std::move(name)}, size{size}, type{std::move(type)} { }

StructField::StructField(cotyl::CString&& name, nested_type_t&& contained) :
    StructField{std::move(name), 0, std::move(contained)} { }

void StructUnionType::ForgetConstInfo() const {
  for (auto& field : fields) {
    (*field.type)->ForgetConstInfo();
  }
}

std::string StructUnionType::BodyString() const {
  cotyl::StringStream repr{};
  if (!name.empty()) {
    repr << name;
  }
  repr << "{";

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

std::string StructType::ToString() const {
  return "struct " + BodyString();
}

std::string UnionType::ToString() const {
  return "union " + BodyString();
}

u64 StructType::Sizeof() const {
  // todo: different for union
  if (fields.empty()) {
    throw std::runtime_error("Cannot get size of incomplete struct definition");
  }

  u64 value = 0;
  for (const auto& field : fields) {
    u64 align = (*field.type)->Alignof();

    // padding
    if (value & (align - 1)) {
      value += align - (value & (align - 1));
    }
    value += (*field.type)->Sizeof();
  }
  return value;
}

u64 UnionType::Sizeof() const {
  throw cotyl::UnimplementedException();
}

AnyType StructUnionType::MemberAccess(const cotyl::CString& member) const {
  for (auto& field : fields) {
    if (field.name == member) {
      return *field.type;
    }
  }
  throw std::runtime_error("No field named " + member.str() + " in " + ToString());
}

AnyType StructUnionType::CommonTypeImpl(const AnyType& other) const {
  // TypeEqual is already checked
  InvalidOperands(this, "operation", other);
}

bool StructUnionType::TypeEqualImpl(const StructUnionType& other) const {
  if (fields.size() != other.fields.size()) {
    return false;
  }

  // incomplete type
  if (fields.empty()) {
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
    if (!(*this_field.type).TypeEquals(*other_field.type)) {
      return false;
    }
    if ((*this_field.type)->qualifiers != (*other_field.type)->qualifiers) {
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