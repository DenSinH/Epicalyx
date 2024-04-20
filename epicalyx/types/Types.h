#pragma once

#include "BaseType.h"
#include "Variant.h"
#include "Packs.h"

#include <type_traits>
#include <optional>

namespace epi::type {

using nested_type_t = std::shared_ptr<AnyType>;


struct VoidType final : BaseType {
  VoidType(u8 flags = 0) : BaseType{LValueNess::None, flags} { }

  u64 Sizeof() const final;

  std::string ToString() const final;
  AnyType CommonTypeImpl(const AnyType& other) const final;
};


struct AnyValueType : BaseType {
  using BaseType::BaseType;
};


using value_type_pack = cotyl::pack<
  i8, u8, 
  i16, u16,
  i32, u32, 
  i64, u64,
  float, double
>;


template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
struct ValueType final : AnyValueType {
  using type_t = T;

  ValueType(LValueNess lvalue, u8 flags = 0) : 
      AnyValueType{lvalue, flags}, value{} { }

  ValueType(T value, LValueNess lvalue, u8 flags = 0) : 
      AnyValueType{lvalue, flags}, value{value} { }

  ValueType(std::optional<T> value, LValueNess lvalue, u8 flags = 0) :
      AnyValueType{lvalue, flags}, value{value} { }

  std::optional<T> value;

  constexpr T Get() const { return value.value(); }
  constexpr bool HasValue() const { return value.has_value(); }
  // bool IsConstexpr() const final { return HasValue(); }
  // bool HasTruthiness() const final { return true; }
  // bool IsIntegral() const final { return std::is_integral_v<T>; }
  // bool IsSigned() const final {
  //   if constexpr(std::is_integral_v<T>) {
  //     return std::is_signed_v<T>;
  //   }
  //   throw std::runtime_error("Type is not integral");
  // }

  // bool GetBoolValue() const final {
  //   if (HasValue()) { return Get() != 0; }
  //   throw std::runtime_error("Bool value requested from non-constant Get");
  // }

  std::string ToString() const final;
  
  AnyType Add(const AnyType& other) const final;
  AnyType Sub(const AnyType& other) const final;
  AnyType Mul(const AnyType& other) const final;
  AnyType Div(const AnyType& other) const final;
  AnyType Mod(const AnyType& other) const final;
  AnyType Xor(const AnyType& other) const final;
  AnyType BinAnd(const AnyType& other) const final;
  AnyType BinOr(const AnyType& other) const final;

  AnyType Lt(const AnyType& other) const final;
  AnyType Eq(const AnyType& other) const final;
  AnyType LShift(const AnyType& other) const final;
  AnyType RShift(const AnyType& other) const final;

  AnyType Pos() const;
  AnyType Neg() const;
  AnyType BinNot() const;

  u64 Sizeof() const final { return sizeof(T); }

  // i64 ConstIntVal() const final {
  //   if constexpr(!std::is_integral_v<T>) {
  //     throw std::runtime_error("Floating point type is not an integral value");
  //   }
  //   else {
  //     if constexpr (std::is_unsigned_v<T>) {
  //       return (std::make_signed_t<T>)Get();
  //     }
  //     return Get();
  //   }
  // }
  AnyType CommonTypeImpl(const AnyType& other) const final;
  void ForgetConstInfo() final { value.reset(); }
};


struct PointerType final : BaseType {
  
  PointerType(nested_type_t&& contained, LValueNess lvalue, std::size_t size = 0, u8 flags = 0) :
      BaseType{lvalue, flags}, contained{std::move(contained)}, size{size} { }

  nested_type_t contained;

  // size == 0 ==> "normal" pointer
  std::size_t size;

  std::string ToString() const final;  //  { return cotyl::FormatStr("(%s)*", contained); }
  // bool HasTruthiness() const final { return true; }

  AnyType Add(const AnyType& other) const final;
  AnyType Sub(const AnyType& other) const final;
  AnyType Lt(const AnyType& other) const final;
  AnyType Eq(const AnyType& other) const final;
  AnyType Deref() const final;

  u64 Sizeof() const final;

  void ForgetConstInfo() final;
  AnyType CommonTypeImpl(const AnyType& other) const final;
};

struct FunctionType final : BaseType {
  FunctionType(
    nested_type_t&& return_type, bool variadic, LValueNess lvalue, u8 flags = 0
  ) : BaseType{lvalue, flags}, return_type{std::move(return_type)}, variadic{variadic} {}

  struct Arg {
    Arg(cotyl::CString&& name, nested_type_t&& type) : 
        name{std::move(name)}, type{std::move(type)} { }

    std::string ToString() const;
    cotyl::CString name;
    nested_type_t type;
  };

  nested_type_t return_type;
  std::vector<Arg> arg_types;
  bool variadic;

  AnyType Deref() const final;

  void AddArg(cotyl::CString&& name, nested_type_t&& arg);
  u64 Sizeof() const final { return sizeof(u64); }

  std::string ToString() const final;
  AnyType FunctionCall(const std::vector<AnyType>& args) const final;
  
  bool TypeEqualImpl(const FunctionType& other) const;
  AnyType CommonTypeImpl(const AnyType& other) const final;
};


struct StructField {
  StructField(cotyl::CString&& name, size_t size, nested_type_t&& contained);
      // name{std::move(name)}, size{size}, type{std::move(type)} { }

  StructField(cotyl::CString&& name, nested_type_t&& contained) :
      StructField{std::move(name), 0, std::move(contained)} { }

  cotyl::CString name;
  std::size_t size = 0;  // 0 means default size
  nested_type_t type;
};


struct StructUnionType : BaseType {
  StructUnionType(cotyl::CString&& name, LValueNess lvalue, u8 flags = 0) :
      name{std::move(name)}, BaseType{lvalue, flags} { }

  cotyl::CString name;
  std::vector<StructField> fields;  // empty if struct was only declared but never defined

  template<typename... Args>
  void AddField(Args&&... args) {
    fields.emplace_back(std::move(args)...);
  }

  AnyType MemberAccess(const cotyl::CString& member) const final;

  // pType<> CastToImpl(const StructUnionType& other) const {
  //   if (_EqualTypeImpl(other)) {
  //     return other.Clone();
  //   }
  //   throw std::runtime_error("Bad struct or union cast");
  // }
  // bool _EqualTypeImpl(const StructUnionType& other) const;

  void ForgetConstInfo() final; 
  AnyType CommonTypeImpl(const AnyType& other) const final;
  bool TypeEqualImpl(const StructUnionType& other) const;

protected:
  std::string BodyString() const;
};


struct StructType final : StructUnionType {
  StructType(cotyl::CString&& name, LValueNess lvalue, u8 flags = 0) :
      StructUnionType{std::move(name), lvalue, flags} { }

  u64 Sizeof() const final;
  std::string ToString() const final;
};


struct UnionType final : StructUnionType {
  UnionType(cotyl::CString&& name, LValueNess lvalue, u8 flags = 0) :
      StructUnionType{std::move(name), lvalue, flags} { }

  u64 Sizeof() const final;
  std::string ToString() const final;
};

}
