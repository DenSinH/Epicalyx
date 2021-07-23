#pragma once

#include <memory>
#include <vector>
#include <string>
#include <sstream>
#include <optional>
#include <stdexcept>
#include <type_traits>

#include "TypeUtils.h"
#include "Default.h"
#include "Format.h"

namespace epi {

struct CType;

template<typename T = CType>
using pType = std::shared_ptr<T>;

template<typename T = CType, typename ...Args>
pType<T> MakeType(Args ...args) {
  return std::make_shared<T>(args...);
}

#ifdef min
#   undef min
#endif
#ifdef max
#   undef max
#endif

template<typename T>
struct ValueType;
struct VoidType;
struct PointerType;
struct ArrayType;
struct FunctionType;
struct StructType;
struct UnionType;

#define INTEGRAL_TYPE_SIGNATURES(MACRO, ...) \
    MACRO(__VA_ARGS__, ValueType<u8>) \
    MACRO(__VA_ARGS__, ValueType<i8>) \
    MACRO(__VA_ARGS__, ValueType<i16>) \
    MACRO(__VA_ARGS__, ValueType<u16>) \
    MACRO(__VA_ARGS__, ValueType<i32>) \
    MACRO(__VA_ARGS__, ValueType<u32>) \
    MACRO(__VA_ARGS__, ValueType<i64>) \
    MACRO(__VA_ARGS__, ValueType<u64>) \

#define NUMERIC_TYPE_SIGNATURES(MACRO, ...) \
    INTEGRAL_TYPE_SIGNATURES(MACRO, __VA_ARGS__) \
    MACRO(__VA_ARGS__, ValueType<float>) \
    MACRO(__VA_ARGS__, ValueType<double>)    \

#define TYPE_SIGNATURES(MACRO, ...) \
    NUMERIC_TYPE_SIGNATURES(MACRO, __VA_ARGS__) \
    MACRO(__VA_ARGS__, VoidType) \
    MACRO(__VA_ARGS__, PointerType) \
    MACRO(__VA_ARGS__, ArrayType) \
    MACRO(__VA_ARGS__, FunctionType) \
    MACRO(__VA_ARGS__, StructType) \
    MACRO(__VA_ARGS__, UnionType)

#define VIRTUAL_BINOP_HANDLERS(handler) TYPE_SIGNATURES(VIRTUAL_BINOP_HANDLER, handler)

#define BINOP_HANDLER(handler, type) pType<> handler(const type& other) const
#define UNOP_HANDLER(handler) pType<> handler() const
#define VIRTUAL_CASTABLE(_, type) virtual bool CastableTypeImpl(const type& other) const { return false; }
#define VIRTUAL_EQ(_, type) virtual bool EqualTypeImpl(const type& other) const { return false; }
#define VIRTUAL_BINOP_HANDLER(handler, type) virtual BINOP_HANDLER(handler, type) { throw std::runtime_error("Invalid types for operand"); }
#define VIRTUAL_BINOP(handler) public: virtual pType<> handler(const CType& other) const { throw std::runtime_error("Unimplemented binop handler"); }; protected: VIRTUAL_BINOP_HANDLERS(R ## handler)
#define VIRTUAL_UNOP(handler) public: virtual UNOP_HANDLER(handler) { throw std::runtime_error("Unimplemented unop handler"); }

struct CType {

  enum Qualifier : u32 {
    Const = 0x1,
    Restrict = 0x2,
    Volatile = 0x4,
    Atomic = 0x8,
  };

  enum class LValueNess {
    None = 0,
    LValue,
    Assignable,
  };

  CType(LValueNess lvalue,
        u32 flags = 0) :
          lvalue(lvalue),
          qualifiers(flags) {

  }

  virtual bool IsConstexpr() const { return false; }  // for optimizing branching

  virtual pType<ValueType<i32>> TruthinessAsCType() const {
    throw std::runtime_error("Type does not have a truthiness value: " + to_string());
  }

  virtual bool GetBoolValue() const {
    throw std::runtime_error("Type cannot be converted to bool");
  }

  static pType<ValueType<i8>> ConstOne();

  virtual std::string to_string() const {
    throw std::runtime_error("Unimplemented ctype");
  };

  VIRTUAL_BINOP(Add)
  VIRTUAL_BINOP(Sub)
  VIRTUAL_BINOP(Mul)
  VIRTUAL_BINOP(Div)
  VIRTUAL_BINOP(Mod)
  VIRTUAL_BINOP(Xor)
  VIRTUAL_BINOP(BinAnd)
  VIRTUAL_BINOP(BinOr)

public:
  pType<ValueType<i32>> LogAnd(const CType& other) const;
  pType<ValueType<i32>> LogOr(const CType& other) const;
  pType<ValueType<i32>> LogNot() const;

  pType<> LLogAnd(const CType& other) const;
  pType<> LLogOr(const CType& other) const;
  pType<> LLogNot() const;

  VIRTUAL_BINOP(Lt)
  VIRTUAL_BINOP(Gt)
  VIRTUAL_BINOP(Eq)
  VIRTUAL_BINOP(ArrayAccess) // todo: constant propagation
  VIRTUAL_BINOP(LShift)
  VIRTUAL_BINOP(RShift)

public:
  // combinations of above functions
  pType<> Le(const CType& other) const;
  pType<> Ge(const CType& other) const;
  pType<> Neq(const CType& other) const;

  virtual pType<> MemberAccess(const std::string& member) const {
    throw std::runtime_error("Cannot access member of non-struct/union type");
  }

  virtual pType<> FunctionCall(const std::vector<pType<>>& args) const {
    throw std::runtime_error("Expression is not a function pointer");
  }

  VIRTUAL_UNOP(Deref)

  UNOP_HANDLER(Ref);  // we want a different default here
  VIRTUAL_UNOP(Neg)
  VIRTUAL_UNOP(Pos)  // really just sets LValueNess to None
  VIRTUAL_UNOP(BinNot)
  UNOP_HANDLER(Incr);
  UNOP_HANDLER(Decr);

  virtual pType<ValueType<u64>> Sizeof() const {
    throw std::runtime_error("Size of incomplete type");
  }

  virtual pType<ValueType<u64>> Alignof() const {
    throw std::runtime_error("Align of incomplete type");
  }

public:
  virtual bool CastableType(const CType& other) const {
    return false;
  }

  // for checking actual equality:
  virtual bool EqualType(const CType& other) const {
    return false;
  }

  bool IsAssignable() const { return lvalue == LValueNess::Assignable; }  // if condition is used to assign to
  virtual bool HasTruthiness() const { return false; }  // if expression is used as condition

  virtual u64 ConstIntVal(bool _signed) const {
    // for array sizes
    throw std::runtime_error("Type is not an integer value: " + to_string());
  }

  virtual pType<> Clone() const = 0;

  u32 qualifiers = 0;

protected:
  enum {
    Struct,
    Union
  };

  LValueNess lvalue;

  void SetNotLValue() { lvalue = LValueNess::None; }         // for example the result of expressions
  void SetLValue() { lvalue = LValueNess::LValue; }          // for example for const function args / struct fields
  void SetAssignable() { lvalue = LValueNess::Assignable; }  // for example for non-const function args / struct fields

  virtual bool IsComplete() const {
    return false;
  }

  virtual void ForgetConstInfo() {
    // forget info on contained value (for example, adding an int to a pointer)
  }

  static pType<ValueType<i32>> MakeBool(bool value);
  static pType<ValueType<i32>> MakeBool();

  TYPE_SIGNATURES(VIRTUAL_CASTABLE)  // compatible types (can cast)
  TYPE_SIGNATURES(VIRTUAL_EQ)        // equal types

private:
  template<typename T>
  friend struct ValueType;
  friend struct VoidType;
  friend struct PointerType;
  friend struct ArrayType;
  friend struct FunctionType;
  template<int>
  friend struct StructUnionType;
  friend struct StructType;
  friend struct UnionType;
};


#define OVERRIDE_BINOP_HANDLER(handler, type) BINOP_HANDLER(handler, type) final
#define OVERRIDE_BINOP_HANDLER_NOIMPL(handler, type) OVERRIDE_BINOP_HANDLER(handler, type);
#define OVERRIDE_BINOP(handler) BINOP_HANDLER(handler, CType) final { return other.R ## handler(*this); }
#define OVERRIDE_UNOP(_operator) pType<> _operator() const final;
#define OVERRIDE_BASE_CASTABLE bool CastableType(const CType& other) const final { return other.CastableType(*this); }
#define OVERRIDE_BASE_EQ bool EqualType(const CType& other) const final { return other.EqualTypeImpl(*this); }


struct VoidType : public CType {
  VoidType(u32 flags = 0) :
          CType(LValueNess::None, flags) {

  }

  bool IsComplete() const final { return false; }
  bool CastableType(const CType& other) const final {
    // any type can be cast to void
    return true;
  }

  OVERRIDE_BASE_EQ

  std::string to_string() const final { return "void"; };

protected:
  bool EqualTypeImpl(const VoidType& other) const final { return true; }
  pType<> Clone() const final { return MakeType<VoidType>(qualifiers); }
};


template<typename T>
struct ValueType : public CType {
  explicit ValueType(LValueNess lvalue = LValueNess::Assignable, u32 flags = 0) :
          CType(lvalue, flags),
          value() {

  }

  explicit ValueType(T value, LValueNess lvalue = LValueNess::Assignable, u32 flags = 0) :
          CType(lvalue, flags),
          value(value) {

  }

  bool GetBoolValue() const final {
    if (value.has_value()) {
      return value.value() != 0;
    }
    throw std::runtime_error("Bool value requested from non-constant Get");
  }

  constexpr T Get() const {
    return value.value();
  }

  constexpr bool HasValue() const {
    return value.has_value();
  }

  bool IsConstexpr() const final { return HasValue(); }
  bool HasTruthiness() const final { return true; }

  std::string to_string() const final {
    if (!value.has_value()) {
      return type_string_v<T>;
    }
    return cotyl::FormatStr("%s:%s", type_string_v<T>, value.value());
  }

  pType<> Clone() const final {
    if (HasValue()) {
      return MakeType<ValueType<T>>(Get(), lvalue, qualifiers);
    }
    return MakeType<ValueType<T>>(lvalue, qualifiers);
  }

  OVERRIDE_BASE_CASTABLE
  OVERRIDE_BASE_EQ

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
  OVERRIDE_BINOP(Gt);
  OVERRIDE_BINOP(Eq);

  OVERRIDE_UNOP(Pos);
  OVERRIDE_UNOP(Neg);
  OVERRIDE_UNOP(BinNot);

  OVERRIDE_BINOP(ArrayAccess);

  pType<ValueType<u64>> Sizeof() const final {
    return MakeType<ValueType<u64>>(sizeof(T), LValueNess::None);
  }

  pType<ValueType<u64>> Alignof() const final {
    // todo:
    return MakeType<ValueType<u64>>(sizeof(T), LValueNess::None);
  }

  std::optional<T> value;

protected:
  pType<ValueType<i32>> TruthinessAsCType() const final {
    if (HasValue()) {
      return MakeBool(value.value() != 0);
    }
    return MakeBool();
  }

  u64 ConstIntVal(bool _signed) const final {
    if constexpr(!std::is_integral_v<T>) {
      throw std::runtime_error("Floating point type is not an integral value");
    }
    if (_signed != std::is_unsigned_v<T>) {
      throw std::runtime_error("Expected " + (_signed ? std::string() : "un") + "signed value");
    }
    return Get();
  }

  void ForgetConstInfo() final {
    value = {};
  }

private:
  // perform BinOp on other in reverse: so this.ValueTypeRBinOp<std::plus> <==> other + this
  template<typename L, template<typename t> class _handler, typename common_t = std::common_type_t<L, T>>
  pType<> ValueTypeRBinOp(const ValueType<L>& other) const {
    // results of binary expressions are never an lvalue
    _handler<common_t> handler;
    if (HasValue() && other.HasValue()) {
      return MakeType<ValueType<common_t>>(handler(other.Get(), Get()), LValueNess::None, 0);
    }
    return MakeType<ValueType<common_t>>(LValueNess::None, 0);
  }

  template<typename L, template<typename t> class _handler, typename common_t = std::common_type_t<L, T>>
  pType<> ValueTypeRBoolBinOp(const ValueType<L>& other) const {
    _handler<common_t> handler;
    if (HasValue() && other.HasValue()) {
      return MakeBool(handler(other.Get(), Get()));
    }
    return MakeBool();
  }

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
  NUMERIC_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RGt)
  OVERRIDE_BINOP_HANDLER(RGt, PointerType);
  NUMERIC_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, REq)
  OVERRIDE_BINOP_HANDLER(REq, PointerType);

  OVERRIDE_BINOP_HANDLER(RArrayAccess, PointerType);

private:
  bool CastableTypeImpl(const ValueType<u8>& other) const final { return true; }
  bool CastableTypeImpl(const ValueType<i8>& other) const final { return true; }
  bool CastableTypeImpl(const ValueType<u16>& other) const final { return true; }
  bool CastableTypeImpl(const ValueType<i16>& other) const final { return true; }
  bool CastableTypeImpl(const ValueType<u32>& other) const final { return true; }
  bool CastableTypeImpl(const ValueType<i32>& other) const final { return true; }
  bool CastableTypeImpl(const ValueType<u64>& other) const final { return true; }
  bool CastableTypeImpl(const ValueType<i64>& other) const final { return true; }
  bool CastableTypeImpl(const ValueType<float>& other) const final { return true; }
  bool CastableTypeImpl(const ValueType<double>& other) const final { return true; }

  bool EqualTypeImpl(const ValueType<T>& other) const final {
    return true;
  }
};


struct PointerType : public CType {
  PointerType(const pType<>& contained, LValueNess lvalue = LValueNess::None, u32 flags = 0) :
      CType(lvalue, flags),
      contained(contained ? contained->Clone() : nullptr) {

  }

  pType<> contained;

  std::string to_string() const override {
    return cotyl::FormatStr("(%s)*", contained ? contained->to_string() : "%%");
  }

  bool HasTruthiness() const final { return true; }
  bool CastableType(const CType& other) const override { return other.CastableType(*this); }
  bool EqualType(const CType& other) const override { return other.EqualTypeImpl(*this); }

  OVERRIDE_BINOP(Add)
  OVERRIDE_BINOP(Sub)

  pType<> ArrayAccess(const CType& other) const override { return other.RArrayAccess(*this); }
  pType<> Deref() const override;

  pType<ValueType<u64>> Sizeof() const final {
    return MakeType<ValueType<u64>>(sizeof(u64), LValueNess::None);
  }

  pType<ValueType<u64>> Alignof() const final {
    // todo:
    return MakeType<ValueType<u64>>(sizeof(u64), LValueNess::None);
  }

  pType<> Clone() const override {
    return MakeType<PointerType>(contained ? contained->Clone() : nullptr, lvalue, qualifiers);
  }

private:
  pType<ValueType<i32>> TruthinessAsCType() const override {
    return MakeBool();
  }

  void ForgetConstInfo() final {
    if (contained) contained->ForgetConstInfo();
  }

  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RAdd)
  // we cannot do `int - ptr_type`, so we must not final this
  OVERRIDE_BINOP_HANDLER(RSub, PointerType);
  OVERRIDE_BINOP_HANDLER(RLt, PointerType);
  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RLt)
  OVERRIDE_BINOP_HANDLER(RGt, PointerType);
  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RGt)
  OVERRIDE_BINOP_HANDLER(REq, PointerType);
  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, REq)
  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RArrayAccess)

  bool CastableTypeImpl(const PointerType&) const override { return true; }
  bool CastableTypeImpl(const ValueType<i8>&) const override { return true; }
  bool CastableTypeImpl(const ValueType<u8>&) const override { return true; }
  bool CastableTypeImpl(const ValueType<i16>&) const override { return true; }
  bool CastableTypeImpl(const ValueType<u16>&) const override { return true; }
  bool CastableTypeImpl(const ValueType<i32>&) const override { return true; }
  bool CastableTypeImpl(const ValueType<u32>&) const override { return true; }
  bool CastableTypeImpl(const ValueType<i64>&) const override { return true; }
  bool CastableTypeImpl(const ValueType<u64>&) const override { return true; }

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

  std::string to_string() const override {
    return cotyl::FormatStr("(%s)[%s]", contained ? contained->to_string() : "%%", size);
  }

  pType<> Clone() const override {
    return MakeType<ArrayType>(contained, size, qualifiers);
  }
};

struct FunctionType : public PointerType {
  FunctionType(
          const pType<>& return_type,
          std::string symbol = "",
          bool variadic = false,
          LValueNess lvalue = LValueNess::LValue,
          u32 flags = 0
  ) :
          PointerType(return_type, lvalue, flags),
          symbol(std::move(symbol)),
          variadic(variadic) {

    if (contained) contained->ForgetConstInfo();
    if (!symbol.empty()) {
      if (lvalue != LValueNess::LValue) {
        throw std::runtime_error("Bad function type initializer (symbol that is not an lvalue)");
      }
    }
    // functions are assignable if they are variables, but not if they are global symbols
  }

  bool variadic;
  const std::string symbol;  // if there is a symbol here, the function is a global definition
  std::vector<pType<>> arg_types;

  void AddArg(const pType<const CType>& arg) {
    arg_types.push_back(arg->Clone());
    arg_types.back()->ForgetConstInfo();  // constant info is nonsense for arguments
  }

  pType<ValueType<i32>> TruthinessAsCType() const override {
    if (!symbol.empty()) {
      return MakeBool(true);  // always true
    }
    return MakeBool();  // unknown, might be function pointer variable
  }

  std::string to_string() const final {
    std::stringstream repr{};
    std::string formatted = cotyl::FormatStr("(%s)%s(", contained ? contained->to_string() : "%%", symbol);
    repr << formatted;
    repr << cotyl::Join(", ", arg_types);
    if (variadic) {
      repr << "...";
    }
    repr << ')';
    return repr.str();
  }

  pType<> Clone() const override {
    auto clone = MakeType<FunctionType>(contained, symbol, variadic, lvalue, qualifiers);
    for (const auto& arg : arg_types) {
      clone->AddArg(arg);
    }
    return clone;
  }

  OVERRIDE_BASE_CASTABLE
  OVERRIDE_BASE_EQ

  OVERRIDE_UNOP(Deref)

  pType<> ArrayAccess(const CType& other) const final {
    throw std::runtime_error("Cannot access elements from function pointer");
  }

  pType<> FunctionCall(const std::vector<pType<>>& args) const final {
    if (args.size() != arg_types.size()) {
      if (!variadic || args.size() < arg_types.size()) {
        throw std::runtime_error("Not enough arguments for function call");
      }
    }

    for (int i = 0; i < arg_types.size(); i++) {
      if (!arg_types[i]->CastableType(*args[i])) {
        throw std::runtime_error("Cannot cast argument type");
      }
    }
    return contained->Clone();
  }

private:
  bool CastableTypeImpl(const FunctionType& other) const final { return true; }
  bool CastableTypeImpl(const PointerType& other) const final { return true; }
  bool CastableTypeImpl(const ValueType<i8>&) const final { return true; }
  bool CastableTypeImpl(const ValueType<u8>&) const final { return true; }
  bool CastableTypeImpl(const ValueType<i16>&) const final { return true; }
  bool CastableTypeImpl(const ValueType<u16>&) const final { return true; }

  bool CastableTypeImpl(const ValueType<i32>&) const final { return true; }
  bool CastableTypeImpl(const ValueType<u32>&) const final { return true; }
  bool CastableTypeImpl(const ValueType<i64>&) const final { return true; }
  bool CastableTypeImpl(const ValueType<u64>&) const final { return true; }

  bool EqualTypeImpl(const PointerType& other) const final { return false; }
  bool EqualTypeImpl(const FunctionType& other) const final {
    if (!(*contained).EqualType(*other.contained)) {
      return false;
    }
    if (arg_types.size() != other.arg_types.size()) {
      return false;
    }

    for (size_t i = 0; i < arg_types.size(); i++) {
      if (!(*arg_types[i]).EqualType(*other.arg_types[i])) {
        return false;
      }
    }

    return true;
  }
};


template<int t>
struct StructUnionType : public CType {
  StructUnionType(LValueNess lvalue, u32 flags = 0) :
          CType(lvalue, flags) {

  }

  struct Field {
    Field(std::string name, size_t size, const pType<>& contained) :
            name(std::move(name)),
            size(size),
            type(contained->Clone()) {

    }

    Field(std::string name, const pType<>& contained) :
            name(std::move(name)),
            size(0),
            type(contained->Clone()) {

    }

    const std::string name;
    const size_t size = 0;  // 0 means default size
    pType<> type;
  };

  void AddField(const std::string& name, size_t size, const pType<>& contained) {
    fields.push_back(Field(name, size, contained));
  }

  void AddField(const std::string& name, const pType<>& contained) {
    fields.push_back(Field(name, contained));
  }

  pType<> MemberAccess(const std::string& member) const final;

  pType<ValueType<u64>> Sizeof() const final {
    if (!IsComplete()) {
      CType::Sizeof();
      return nullptr;
    }
    u64 value = 0;
    for (const auto& field : fields) {
      value += field.type->Sizeof()->Get();
    }
    return MakeType<ValueType<u64>>(value, LValueNess::None);
  }

  pType<ValueType<u64>> Alignof() const final {
    // todo:
    return Sizeof();
  }

  pType<> Clone() const final {
    auto clone = MakeType<StructUnionType<t>>(lvalue, qualifiers);
    for (const auto& arg : fields) {
      clone->AddField(arg.name, arg.size, arg.type);
    }
    return clone;
  }

  std::vector<Field> fields;  // empty if struct was only declared but never defined

protected:
  bool _CastableTypeImpl(const StructUnionType<t>& other) const {
    return _EqualTypeImpl(other);
  }

  bool _EqualTypeImpl(const StructUnionType<t>& other) const {
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

  bool IsComplete() const final {
    return !fields.empty();
  }

  void ForgetConstInfo() final {
    for (auto& field : fields) {
      field.type->ForgetConstInfo();
    }
  }
};

struct StructType : public StructUnionType<CType::Struct> {
  StructType(LValueNess lvalue, u32 flags = 0) :
          StructUnionType(lvalue, flags) {

  }

  std::string to_string() const final {
    return "struct";
  }

  OVERRIDE_BASE_CASTABLE
  OVERRIDE_BASE_EQ

private:
  bool CastableTypeImpl(const StructType& other) const final {
    return _CastableTypeImpl(other);
  }

  bool EqualTypeImpl(const StructType& other) const final {
    return _EqualTypeImpl(other);
  }
};

struct UnionType : public StructUnionType<CType::Union> {
  UnionType(LValueNess lvalue, u32 flags = 0) :
          StructUnionType(lvalue, flags) {

  }

  std::string to_string() const final {
    return "union";
  }

  OVERRIDE_BASE_CASTABLE
  OVERRIDE_BASE_EQ

private:
  bool CastableTypeImpl(const UnionType& other) const final {
    return _CastableTypeImpl(other);
  }

  bool EqualTypeImpl(const UnionType& other) const final {
    return _EqualTypeImpl(other);
  }
};

}
