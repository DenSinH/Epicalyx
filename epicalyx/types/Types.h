#pragma once

#include "BaseType.h"
#include "TypeFwd.h"
#include "Variant.h"
#include "Packs.h"

#include <type_traits>
#include <optional>

namespace epi::type {

using nested_type_t = std::shared_ptr<const AnyType>;


struct VoidType final : BaseType {
  VoidType(u8 flags = 0) : BaseType{LValue::None, flags} { }

  u64 Sizeof() const final;

  std::string ToString() const final;
  AnyType CommonTypeImpl(const AnyType& other) const final;
};


struct AnyValueType : BaseType {
  using BaseType::BaseType;
};


template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
struct ValueType final : AnyValueType {
  using type_t = T;

  ValueType(LValue lvalue, u8 flags = 0) : 
      AnyValueType{lvalue, flags}, value{} { }

  ValueType(T value, LValue lvalue, u8 flags = 0) : 
      AnyValueType{lvalue, flags}, value{value} { }

  ValueType(std::optional<T> value, LValue lvalue, u8 flags = 0) :
      AnyValueType{lvalue, flags}, value{value} { }

  mutable std::optional<T> value;

  constexpr T Get() const { return value.value(); }
  constexpr bool HasValue() const { return value.has_value(); }

  std::string ToString() const final;
  
  AnyType Add(const AnyType& other) const final;
  AnyType Sub(const AnyType& other) const final;
  AnyType Mul(const AnyType& other) const final;
  AnyType Div(const AnyType& other) const final;
  AnyType Mod(const AnyType& other) const final;
  AnyType Xor(const AnyType& other) const final;
  AnyType BinAnd(const AnyType& other) const final;
  AnyType BinOr(const AnyType& other) const final;

  BoolType Lt(const AnyType& other) const final;
  BoolType Eq(const AnyType& other) const final;
  AnyType LShift(const AnyType& other) const final;
  AnyType RShift(const AnyType& other) const final;

  AnyType Pos() const;
  AnyType Neg() const;
  AnyType BinNot() const;

  BoolType Truthiness() const final;

  u64 Sizeof() const final { return sizeof(T); }
  AnyType CommonTypeImpl(const AnyType& other) const final;
  void ForgetConstInfo() const final { value.reset(); }
};


struct AnyPointerType : BaseType {
  virtual ~AnyPointerType() = default;

  AnyPointerType(nested_type_t&& contained, LValue lvalue, u8 flags = 0) :
      BaseType{lvalue, flags}, contained{std::move(contained)} {

  } 

  nested_type_t contained;
};

struct DataPointerType : AnyPointerType {
  using AnyPointerType::AnyPointerType;

  AnyType Sub(const AnyType& other) const final;
  BoolType Lt(const AnyType& other) const final;
  BoolType Eq(const AnyType& other) const final;
  AnyType Deref() const final;

  BoolType Truthiness() const final;
  u64 Stride() const;
};

struct ArrayType final : DataPointerType {

  ArrayType(nested_type_t&& contained, std::size_t size) :
      DataPointerType{std::move(contained), LValue::LValue, Qualifier::Const}, size{size} { 
    // todo: typecheck this
    // if (!(*this->contained)->Sizeof()) {
    //   throw TypeError("Incomplete type for array");
    // }
  }

  // size == 0 ==> unspecified size
  std::size_t size;

  AnyType Add(const AnyType& other) const final;
  std::string ToString() const final;

  u64 Sizeof() const final;
  AnyType CommonTypeImpl(const AnyType& other) const final;
};

struct PointerType final : DataPointerType {

  PointerType(nested_type_t&& contained, LValue lvalue, u8 flags = 0) :
      DataPointerType{std::move(contained), lvalue, flags} { }

  PointerType(const ArrayType& arr) :
      PointerType(nested_type_t{arr.contained}, arr.lvalue, arr.qualifiers) { }

  std::string ToString() const final;

  AnyType Add(const AnyType& other) const final;
  u64 Sizeof() const final;

  AnyType FunctionCall(const cotyl::vector<AnyType>& args) const final;
  
  void ForgetConstInfo() const final;
  AnyType CommonTypeImpl(const AnyType& other) const final;
};

struct FunctionType final : AnyPointerType {
  FunctionType(
    nested_type_t&& return_type, bool variadic, LValue lvalue, u8 flags = 0
  ) : AnyPointerType{std::move(return_type), lvalue, flags}, variadic{variadic} {}

  struct Arg {
    Arg(cotyl::CString&& name, nested_type_t&& type) : 
        name{std::move(name)}, type{std::move(type)} { }

    std::string ToString() const;
    cotyl::CString name;
    nested_type_t type;
  };

  cotyl::vector<Arg> arg_types{};
  bool variadic = false;

  AnyType Deref() const final;

  void AddArg(cotyl::CString&& name, nested_type_t&& arg);
  BoolType Truthiness() const final;
  u64 Sizeof() const final { return sizeof(u64); }

  std::string ToString() const final;
  AnyType FunctionCall(const cotyl::vector<AnyType>& args) const final;
  
  bool TypeEqualImpl(const FunctionType& other) const;
  AnyType CommonTypeImpl(const AnyType& other) const final;
};


struct StructField {
  StructField(cotyl::CString&& name, size_t size, nested_type_t&& contained);
  StructField(cotyl::CString&& name, nested_type_t&& contained);

  cotyl::CString name;
  std::size_t size = 0;  // 0 means default size
  nested_type_t type;
};


struct StructUnionType : BaseType {
  StructUnionType(
    cotyl::CString&& name,
    cotyl::vector<StructField>&& fields,
    LValue lvalue, 
    u8 flags = 0
  ) : BaseType{lvalue, flags}, 
      name{std::move(name)}, 
      fields{std::move(fields)} { 

  }

  cotyl::CString name;
  cotyl::vector<StructField> fields{};  // empty if struct was only declared but never defined
  AnyType MemberAccess(const cotyl::CString& member) const final;

  void ForgetConstInfo() const final; 
  AnyType CommonTypeImpl(const AnyType& other) const final;
  bool TypeEqualImpl(const StructUnionType& other) const;

protected:
  std::string BodyString() const;
};


struct StructType final : StructUnionType {
  StructType(
    cotyl::CString&& name,
    cotyl::vector<StructField>&& fields,
    LValue lvalue, 
    u8 flags = 0
  ) : StructUnionType{
      std::move(name), 
      std::move(fields), 
      lvalue, 
      flags
  } { }

  u64 Sizeof() const final;
  std::string ToString() const final;
};


struct UnionType final : StructUnionType {
  UnionType(
    cotyl::CString&& name,
    cotyl::vector<StructField>&& fields,
    LValue lvalue, 
    u8 flags = 0
  ) : StructUnionType{
      std::move(name), 
      std::move(fields), 
      lvalue, 
      flags
  } { }

  u64 Sizeof() const final;
  std::string ToString() const final;
};

}
