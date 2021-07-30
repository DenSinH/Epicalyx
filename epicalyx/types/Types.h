#pragma once

#include "EpiCType.h"

#include <type_traits>
#include <sstream>
#include <optional>

#include "TypeUtils.h"

namespace epi {

#define OVERRIDE_BINOP_HANDLER(handler, type) BINOP_HANDLER(handler, type) final
#define OVERRIDE_BINOP_HANDLER_NOIMPL(handler, type) OVERRIDE_BINOP_HANDLER(handler, type);
#define OVERRIDE_BINOP(handler) BINOP_HANDLER(handler, CType) final { return other.R ## handler(*this); }
#define OVERRIDE_UNOP(_operator) pType<> _operator() const final;
#define OVERRIDE_BASE_CASTABLE pType<> DoCast(const CType& other) const final { return other.CastTo(*this); }
#define OVERRIDE_BASE_EQ bool EqualType(const CType& other) const final { return other.EqualTypeImpl(*this); }
#define OVERRIDE_BASE_CASTABLE_NOFINAL pType<> DoCast(const CType& other) const override { return other.CastTo(*this); }
#define OVERRIDE_BASE_EQ_NOFINAL bool EqualType(const CType& other) const override { return other.EqualTypeImpl(*this); }


struct VoidType final : public CType {
  VoidType(u32 flags = 0) :
          CType(LValueNess::None, flags) {

  }

  OVERRIDE_BASE_EQ

  bool IsComplete() const final { return false; }
  pType<> DoCast(const CType& other) const final { return MakeType<VoidType>(); }
  void Visit(TypeVisitor& v) const final { v.Visit(*this); }

  std::string ToString() const final { return "void"; };

protected:
  bool EqualTypeImpl(const VoidType& other) const final { return true; }
  pType<> Clone() const final { return MakeType<VoidType>(qualifiers); }
};


template<typename T>
struct ValueType final : public CType {
  explicit ValueType(LValueNess lvalue, u32 flags = 0) :
          CType(lvalue, flags),
          value() {

  }

  explicit ValueType(T value, LValueNess lvalue, u32 flags = 0) :
          CType(lvalue, flags),
          value(value) {

  }

  explicit ValueType(std::optional<T> value, LValueNess lvalue, u32 flags = 0) :
          CType(lvalue, flags),
          value(value) {

  }

  std::optional<T> value;

  OVERRIDE_BASE_CASTABLE
  OVERRIDE_BASE_EQ

  constexpr T Get() const { return value.value(); }
  constexpr bool HasValue() const { return value.has_value(); }
  bool IsConstexpr() const final { return HasValue(); }
  bool HasTruthiness() const final { return true; }
  bool IsIntegral() const final { return std::is_integral_v<T>; }
  bool IsSigned() const final {
    if constexpr(std::is_integral_v<T>) {
      return std::is_signed_v<T>;
    }
    throw std::runtime_error("Type is not integral");
  }
  void Visit(TypeVisitor& v) const final { v.Visit(*this); }

  bool GetBoolValue() const final {
    if (HasValue()) {
      return Get() != 0;
    }
    throw std::runtime_error("Bool value requested from non-constant Get");
  }

  std::string ToString() const final {
    if (!HasValue()) {
      return type_string_v<T>;
    }
    return cotyl::FormatStr("%s:%s", type_string_v<T>, Get());
  }

  pType<> Clone() const final {
    if (HasValue()) {
      return MakeType<ValueType<T>>(Get(), lvalue, qualifiers);
    }
    return MakeType<ValueType<T>>(lvalue, qualifiers);
  }

  OVERRIDE_BINOP(Add)
  OVERRIDE_BINOP(Sub)
  OVERRIDE_BINOP(Mul);
  OVERRIDE_BINOP(Div);
  OVERRIDE_BINOP(Mod);
  OVERRIDE_BINOP(Xor);

  OVERRIDE_BINOP(BinAnd);
  OVERRIDE_BINOP(BinOr);
  OVERRIDE_BINOP(LShift);
  OVERRIDE_BINOP(RShift);

  OVERRIDE_BINOP(Lt);
  OVERRIDE_BINOP(Eq);

  OVERRIDE_UNOP(Pos);
  OVERRIDE_UNOP(Neg);
  OVERRIDE_UNOP(BinNot);

  OVERRIDE_BINOP(CommonType);
  u64 Sizeof() const final { return sizeof(T); }

  i64 ConstIntVal() const final {
    if constexpr(!std::is_integral_v<T>) {
      throw std::runtime_error("Floating point type is not an integral value");
    }
    else {
      if constexpr (std::is_unsigned_v<T>) {
        return (std::make_signed_t<T>)Get();
      }
      return Get();
    }
  }

  void ForgetConstInfo() final { value = {}; }

private:
  // perform BinOp on other in reverse: so this.ValueTypeRBinOp<std::plus> <==> other + this
  template<typename L, template<typename t> class _handler, typename common_t = std::common_type_t<L, T>>
  pType<> ValueTypeRBinOp(const ValueType<L>& other) const;

  template<typename L, template<typename t> class _handler, typename common_t = std::common_type_t<L, T>>
  pType<> ValueTypeRBoolBinOp(const ValueType<L>& other) const;

  NUMERIC_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RAdd)
  OVERRIDE_BINOP_HANDLER(RAdd, PointerType);
  NUMERIC_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RSub)
  OVERRIDE_BINOP_HANDLER(RSub, PointerType);

  NUMERIC_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RMul)
  NUMERIC_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RDiv)
  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RMod)

  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RLShift)
  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RRShift)

  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RXor)
  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RBinAnd)
  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RBinOr)

  NUMERIC_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RLt)
  OVERRIDE_BINOP_HANDLER(RLt, PointerType);
  NUMERIC_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, REq)
  OVERRIDE_BINOP_HANDLER(REq, PointerType);

  NUMERIC_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RCommonType)
  OVERRIDE_BINOP_HANDLER(RCommonType, PointerType);

private:
  pType<> CastTo(const ValueType<u8>& other) const final  { return MakeType<ValueType<u8>>(value, LValueNess::None, other.qualifiers); }
  pType<> CastTo(const ValueType<i8>& other) const final  { return MakeType<ValueType<i8>>(value, LValueNess::None, other.qualifiers); }
  pType<> CastTo(const ValueType<u16>& other) const final { return MakeType<ValueType<u16>>(value, LValueNess::None, other.qualifiers); }
  pType<> CastTo(const ValueType<i16>& other) const final { return MakeType<ValueType<i16>>(value, LValueNess::None, other.qualifiers); }
  pType<> CastTo(const ValueType<u32>& other) const final { return MakeType<ValueType<u32>>(value, LValueNess::None, other.qualifiers); }
  pType<> CastTo(const ValueType<i32>& other) const final { return MakeType<ValueType<i32>>(value, LValueNess::None, other.qualifiers); }
  pType<> CastTo(const ValueType<u64>& other) const final { return MakeType<ValueType<u64>>(value, LValueNess::None, other.qualifiers); }
  pType<> CastTo(const ValueType<i64>& other) const final { return MakeType<ValueType<i64>>(value, LValueNess::None, other.qualifiers); }
  pType<> CastTo(const ValueType<float>& other) const final { return MakeType<ValueType<float>>(value, LValueNess::None, other.qualifiers); }
  pType<> CastTo(const ValueType<double>& other) const final { return MakeType<ValueType<double>>(value, LValueNess::None, other.qualifiers); }
  pType<> CastTo(const PointerType& other) const final { return other.Clone(); }

  bool EqualTypeImpl(const ValueType<T>& other) const final { return true; }
};


struct PointerType : public CType {
  PointerType(const pType<>& contained, LValueNess lvalue, u32 flags = 0) :
      CType(lvalue, flags),
      contained(contained ? contained->Clone() : nullptr) {

  }

  pType<> contained;

  OVERRIDE_BASE_CASTABLE_NOFINAL
  OVERRIDE_BASE_EQ_NOFINAL

  std::string ToString() const override { return cotyl::FormatStr("(%s)*", contained); }
  bool HasTruthiness() const final { return true; }

  OVERRIDE_BINOP(Add)
  OVERRIDE_BINOP(Sub)
  pType<> Deref() const override;

  OVERRIDE_BINOP(CommonType)
  u64 Sizeof() const override { return sizeof(u64); }
  bool IsPointer() const final { return true; }

  pType<> Clone() const override {
    return MakeType<PointerType>(contained ? contained->Clone() : nullptr, lvalue, qualifiers);
  }
  void Visit(TypeVisitor& v) const override { v.Visit(*this); }

private:
  void ForgetConstInfo() override { if (contained) contained->ForgetConstInfo(); }

  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RAdd)
  // we cannot do `int - ptr_type`, so we must not override this
  OVERRIDE_BINOP_HANDLER(RSub, PointerType);
  OVERRIDE_BINOP_HANDLER(RLt, PointerType);
  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RLt)
  OVERRIDE_BINOP_HANDLER(REq, PointerType);
  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, REq)
  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RCommonType)

  pType<> CastTo(const PointerType& other) const override { return other.Clone(); }
  pType<> CastTo(const ValueType<i8>& other) const override { return other.Clone(); }
  pType<> CastTo(const ValueType<u8>& other) const override { return other.Clone(); }
  pType<> CastTo(const ValueType<i16>& other) const override { return other.Clone(); }
  pType<> CastTo(const ValueType<u16>& other) const override { return other.Clone(); }
  pType<> CastTo(const ValueType<i32>& other) const override { return other.Clone(); }
  pType<> CastTo(const ValueType<u32>& other) const override { return other.Clone(); }
  pType<> CastTo(const ValueType<i64>& other) const override { return other.Clone(); }
  pType<> CastTo(const ValueType<u64>& other) const override { return other.Clone(); }

  bool EqualTypeImpl(const PointerType& other) const override {
    return (*contained).EqualType(*other.contained);
  }
};


struct ArrayType : public PointerType {
  ArrayType(const pType<>& contained, size_t size, u32 flags = 0) :
          PointerType(contained, LValueNess::LValue, flags),
          size(size) {

  }

  size_t size;

  bool IsArray() const final { return true; }
  u64 Sizeof() const final { return size * contained->Sizeof(); }

  std::string ToString() const override {
    return cotyl::FormatStr("(%s)[%s]", contained, size);
  }

  pType<> Clone() const override {
    return MakeType<ArrayType>(contained, size, qualifiers);
  }
  void Visit(TypeVisitor& v) const final { v.Visit(*this); }
};

struct FunctionType : public PointerType {
  FunctionType(
          const pType<>& return_type,
          bool variadic,
          LValueNess lvalue,
          u32 flags = 0
  ) :
          PointerType(return_type, lvalue, flags),
          variadic(variadic) {

    if (contained) contained->ForgetConstInfo();
    // functions are assignable if they are variables, but not if they are global symbols
  }

  struct Arg {
    Arg(std::string name, pType<const CType> type) : name(std::move(name)), type(std::move(type)) { }
    std::string ToString() const;
    const std::string name;
    pType<const CType> type;
  };

  bool variadic;
  std::vector<Arg> arg_types;

  OVERRIDE_BASE_CASTABLE
  OVERRIDE_BASE_EQ

  void AddArg(std::string name, const pType<const CType>& arg) {
    auto _arg = arg->Clone();
    _arg->ForgetConstInfo();
    arg_types.emplace_back(std::move(name), _arg);  // constant info is nonsense for arguments
  }

  std::string ToString() const final;
  bool IsFunction() const final { return true; }
  pType<> Clone() const final;
  void Visit(TypeVisitor& v) const final { v.Visit(*this); }

  OVERRIDE_UNOP(Deref)

  pType<> FunctionCall(const std::vector<pType<const CType>>& args) const final;

private:
  pType<> CastTo(const FunctionType& other) const final { return other.Clone(); }
  pType<> CastTo(const PointerType& other) const final { return other.Clone(); }
  pType<> CastTo(const ValueType<i8>& other) const final { return other.Clone(); }
  pType<> CastTo(const ValueType<u8>& other) const final { return other.Clone(); }
  pType<> CastTo(const ValueType<i16>& other) const final { return other.Clone(); }
  pType<> CastTo(const ValueType<u16>& other) const final { return other.Clone(); }
  pType<> CastTo(const ValueType<i32>& other) const final { return other.Clone(); }
  pType<> CastTo(const ValueType<u32>& other) const final { return other.Clone(); }
  pType<> CastTo(const ValueType<i64>& other) const final { return other.Clone(); }
  pType<> CastTo(const ValueType<u64>& other) const final { return other.Clone(); }

  bool EqualTypeImpl(const PointerType& other) const final { return false; }
  bool EqualTypeImpl(const FunctionType& other) const final;
};


struct StructField {
  StructField(std::string name, size_t size, const pType<>& contained) :
          name(std::move(name)),
          size(size),
          type(contained->Clone()) {

  }

  StructField(std::string name, const pType<>& contained) :
          name(std::move(name)),
          size(0),
          type(contained->Clone()) {

  }

  const std::string name;
  const size_t size = 0;  // 0 means default size
  pType<> type;
};


struct StructUnionType : public CType {
  StructUnionType(std::string name, LValueNess lvalue, u32 flags = 0) :
          name(std::move(name)),
          CType(lvalue, flags) {

  }

  const std::string name;
  std::vector<StructField> fields;  // empty if struct was only declared but never defined

  void AddField(const std::string& _name, size_t size, const pType<>& contained) {
    fields.emplace_back(_name, size, contained);
  }

  void AddField(const std::string& _name, const pType<>& contained) {
    fields.emplace_back(_name, contained);
  }

  std::string ToString() const final;
  pType<> MemberAccess(const std::string& member) const final;

  u64 Sizeof() const final;
  bool IsStructlike() const final { return true; }

protected:
  virtual std::string BaseString() const = 0;
  pType<> CastToImpl(const StructUnionType& other) const {
    if (_EqualTypeImpl(other)) {
      return other.Clone();
    }
    throw std::runtime_error("Bad struct or union cast");
  }

  bool _EqualTypeImpl(const StructUnionType& other) const;

  bool IsComplete() const final {
    return !fields.empty();
  }

  void ForgetConstInfo() final {
    for (auto& field : fields) {
      field.type->ForgetConstInfo();
    }
  }
};


struct StructType : public StructUnionType {
  StructType(std::string name, LValueNess lvalue, u32 flags = 0) :
          StructUnionType(std::move(name), lvalue, flags) {

  }

  OVERRIDE_BASE_CASTABLE
  OVERRIDE_BASE_EQ

  pType<> Clone() const final {
    auto clone = MakeType<StructType>(name, lvalue, qualifiers);
    for (const auto& arg : fields) {
      clone->AddField(arg.name, arg.size, arg.type->Clone());
    }
    return clone;
  }
  void Visit(TypeVisitor& v) const final { v.Visit(*this); }

protected:
  std::string BaseString() const final { return "struct"; }

private:
  pType<> CastTo(const StructType& other) const final { return CastToImpl(other); }
  bool EqualTypeImpl(const StructType& other) const final { return _EqualTypeImpl(other); }
};


struct UnionType : public StructUnionType {
  UnionType(std::string name, LValueNess lvalue, u32 flags = 0) :
          StructUnionType(std::move(name), lvalue, flags) {

  }

  OVERRIDE_BASE_CASTABLE
  OVERRIDE_BASE_EQ

  pType<> Clone() const final {
    auto clone = MakeType<UnionType>(name, lvalue, qualifiers);
    for (const auto& arg : fields) {
      clone->AddField(arg.name, arg.size, arg.type->Clone());
    }
    return clone;
  }
  void Visit(TypeVisitor& v) const final { v.Visit(*this); }

protected:
  std::string BaseString() const final { return "struct"; }

private:
  pType<> CastTo(const UnionType& other) const final { return CastToImpl(other); }
  bool EqualTypeImpl(const UnionType& other) const final { return _EqualTypeImpl(other); }
};

}
