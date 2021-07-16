#pragma once

#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <stdexcept>
#include <type_traits>

#include "TypeUtils.h"
#include "Default.h"
#include "Format.h"

namespace epi {

struct CType;

template<typename T = CType>
using Type = std::shared_ptr<T>;

template<typename T = CType, typename ...Args>
Type<T> MakeType(Args ...args) {
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

#define BINOP_HANDLER(handler, type) Type<> handler(const type& other) const
#define UNOP_HANDLER(handler) Type<> handler() const
#define VIRTUAL_CASTABLE(_, type) virtual bool CastableTypeImpl(const type& other) const { return false; }
#define VIRTUAL_EQ(_, type) virtual bool EqualTypeImpl(const type& other) const { return false; }
#define VIRTUAL_BINOP_HANDLER(handler, type) virtual BINOP_HANDLER(handler, type) { throw std::runtime_error("Invalid types for operand"); }
#define VIRTUAL_BINOP(handler) public: virtual Type<> handler(const CType& other) const { throw std::runtime_error("Unimplemented binop handler"); }; protected: VIRTUAL_BINOP_HANDLERS(R ## handler)
#define VIRTUAL_UNOP(handler) public: virtual UNOP_HANDLER(handler) { throw std::runtime_error("Unimplemented unop handler"); }

struct CType {

  enum StorageClass {
    Typedef,
    Extern,
    Static,
    ThreadLocal,
    Register,
    Auto,
  };

  enum QualifierFlags : unsigned {
    QualifierConst = 0x1,
    QualifierRestrict = 0x2,
    QualifierVolatile = 0x4,
    QualifierAtomic = 0x8,
  };

  enum class LValueNess {
    None = 0,
    LValue,
    Assignable,
  };

  CType(LValueNess lvalue,
        unsigned flags = 0,
        StorageClass storage = StorageClass::Auto) :
          lvalue(lvalue),
          storage(storage),
          qualifiers(flags) {

  }

  virtual bool IsConstexpr() const { return false; }  // for optimizing branching

  virtual Type<ValueType<i32>> TruthinessAsCType() const {
    throw std::runtime_error("Type does not have a truthiness value: " + ToString());
  }

  virtual bool GetBoolValue() const {
    throw std::runtime_error("Type cannot be converted to bool");
  }

  static Type<ValueType<i8>> ConstOne();

  virtual std::string ToString() const {
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

  Type<ValueType<i32>> LogAnd(const CType& other) const;
  Type<ValueType<i32>> LogOr(const CType& other) const;
  Type<ValueType<i32>> LogNot() const;

  VIRTUAL_BINOP(Lt)
  VIRTUAL_BINOP(Gt)
  VIRTUAL_BINOP(Eq)
  VIRTUAL_BINOP(ArrayAccess) // todo: constant propagation
  VIRTUAL_BINOP(LShift)
  VIRTUAL_BINOP(RShift)

public:
  // combinations of above functions
  Type<> Le(const CType& other) const;
  Type<> Ge(const CType& other) const;
  Type<> Neq(const CType& other) const;

  virtual Type<> MemberAccess(const std::string& member) const {
    throw std::runtime_error("Cannot access member of non-struct/union type");
  }

  virtual Type<> FunctionCall(const std::vector<Type<>>& args) const {
    throw std::runtime_error("Expression is not a function pointer");
  }

  VIRTUAL_UNOP(Deref)

  virtual UNOP_HANDLER(Ref);  // we want a different default here
  VIRTUAL_UNOP(Neg)
  VIRTUAL_UNOP(Pos)  // really just sets LValueNess to None
  VIRTUAL_UNOP(BinNot)

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
    throw std::runtime_error("Type is not an integer value: " + ToString());
  }

  virtual Type<> Clone() const = 0;

  const StorageClass storage;
  unsigned qualifiers = 0;

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

  static Type<ValueType<i32>> MakeBool(bool value);
  static Type<ValueType<i32>> MakeBool();

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
#define OVERRIDE_UNOP(_operator) Type<> _operator() const final;
#define OVERRIDE_BASE_CASTABLE bool CastableType(const CType& other) const final { return other.CastableType(*this); }
#define OVERRIDE_BASE_EQ bool EqualType(const CType& other) const final { return other.EqualTypeImpl(*this); }

struct VoidType : public CType {
  VoidType(unsigned flags = 0, StorageClass storage = StorageClass::Auto) :
          CType(LValueNess::None, flags, storage) {

  }

  bool IsComplete() const final {
    return false;
  }

  bool CastableType(const CType& other) const final {
    // any type can be cast to void
    return true;
  }

  OVERRIDE_BASE_EQ

  std::string ToString() const final {
    return "void";
  };

protected:
  bool EqualTypeImpl(const VoidType& other) const final {
    return true;
  }

  Type<> Clone() const final {
    return MakeType<VoidType>(qualifiers, storage);
  }
};

template<typename T>
struct ValueType : public CType {
  ValueType(LValueNess lvalue, unsigned flags = 0, StorageClass storage = StorageClass::Auto) :
          CType(lvalue, flags, storage),
          Value() {

  }

  ValueType(T value, LValueNess lvalue, unsigned flags = 0, StorageClass storage = StorageClass::Auto) :
          CType(lvalue, flags, storage),
          Value(value) {

  }

  bool GetBoolValue() const final {
    if (Value.has_value()) {
      return Value.value() != 0;
    }
    throw std::runtime_error("Bool value requested from non-constant Get");
  }

  constexpr T Get() const {
    return Value.value();
  }

  constexpr bool HasValue() const {
    return Value.has_value();
  }

  bool IsConstexpr() const final { return HasValue(); }

  bool HasTruthiness() const final { return true; }

  std::string ToString() const final {
    if (!Value.has_value()) {
      return type_string_v<T>;
    }
    return calyx::Format("%s:%s", type_string_v<T>.c_str(), std::to_string(Value.value()).c_str());
  }

  Type<> Clone() const final {
    if (HasValue()) {
      return MakeType<ValueType<T>>(Get(), lvalue, qualifiers, storage);
    }
    return MakeType<ValueType<T>>(lvalue, qualifiers, storage);
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

  std::optional<T> Value;

protected:
  Type<ValueType<i32>> TruthinessAsCType() const final {
    if (HasValue()) {
      return MakeBool(Value.value() != 0);
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
    Value = {};
  }

private:
  // perform BinOp on other in reverse: so this.ValueTypeRBinOp<std::plus> <==> other + this
  template<typename L, template<typename t> class _handler, typename common_t = std::common_type_t<L, T>>
  Type<> ValueTypeRBinOp(const ValueType<L>& other) const {
    // results of binary expressions are never an lvalue
    _handler<common_t> handler;
    if (HasValue() && other.HasValue()) {
      return MakeType<ValueType<common_t>>(handler(other.Get(), Get()), LValueNess::None, 0);
    }
    return MakeType<ValueType<common_t>>(LValueNess::None, 0);
  }

  template<typename L, template<typename t> class _handler, typename common_t = std::common_type_t<L, T>>
  Type<> ValueTypeRBoolBinOp(const ValueType<L>& other) const {
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
  PointerType(const Type<>& contained, LValueNess lvalue, unsigned flags = 0, StorageClass storage = StorageClass::Auto)
          :
          CType(lvalue, flags, storage),
          contained(contained->Clone()) {

  }

  const Type<> contained;

  std::string ToString() const override {
    return calyx::Format("(%s)*", contained->ToString().c_str());
  }

  bool HasTruthiness() const final { return true; }
  bool CastableType(const CType& other) const override { return other.CastableType(*this); }
  bool EqualType(const CType& other) const override { return other.EqualTypeImpl(*this); }

  OVERRIDE_BINOP(Add)
  OVERRIDE_BINOP(Sub)

  Type<> ArrayAccess(const CType& other) const override { return other.RArrayAccess(*this); }
  Type<> Deref() const override;

  Type<> Clone() const override {
    return MakeType<PointerType>(contained->Clone(), lvalue, qualifiers, storage);
  }

private:
  Type<ValueType<i32>> TruthinessAsCType() const override {
    return MakeBool();
  }

  void ForgetConstInfo() final {
    contained->ForgetConstInfo();
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
  ArrayType(const Type<>& contained, const Type<>& size, unsigned flags = 0, StorageClass storage = StorageClass::Auto) :
          PointerType(contained, LValueNess::LValue, flags, storage),
          size(size->ConstIntVal(false)) {
    // arrays are lvalues, but not assignable (besides the initializer)
  }

  ArrayType(const Type<>& contained, size_t size, unsigned flags = 0, StorageClass storage = StorageClass::Auto) :
          PointerType(contained, LValueNess::LValue, flags, storage),
          size(size) {

  }

  size_t size;

  std::string ToString() const override {
    return calyx::Format("(%s)[%d]", contained->ToString().c_str(), size);
  }

  Type<> Clone() const override {
    return MakeType<ArrayType>(contained, size, qualifiers, storage);
  }
};

struct FunctionType : public PointerType {
  FunctionType(
          const Type<>& return_type,
          std::string symbol = "",
          bool variadic = false,
          unsigned flags = 0,
          StorageClass storage = StorageClass::Auto
  ) :
          PointerType(
                  return_type,
                  symbol.empty() ? LValueNess::Assignable : LValueNess::LValue,
                  flags,
                  storage
          ),
          symbol(std::move(symbol)),
          variadic(variadic) {
    contained->ForgetConstInfo();
    // functions are assignable if they are variables, but not if they are global symbols
  }

  bool variadic;
  const std::string symbol;  // if there is a symbol here, the function is a global definition
  std::vector<Type<>> arg_types;

  void AddArg(const Type<>& arg) {
    arg_types.push_back(arg->Clone());
    arg_types.back()->ForgetConstInfo();  // constant info is nonsense for arguments
  }

  Type<ValueType<i32>> TruthinessAsCType() const override {
    if (!symbol.empty()) {
      return MakeBool(true);  // always true
    }
    return MakeBool();  // unknown, might be function pointer variable
  }

  std::string ToString() const final {
    std::string repr = contained->ToString() + " " + symbol + "(";
    for (auto& a : arg_types) {
      repr += a->ToString() + ",";
    }
    return repr + ")";
  }

  Type<> Clone() const override {
    auto clone = MakeType<FunctionType>(contained, symbol, variadic, qualifiers, storage);
    for (const auto& arg : arg_types) {
      clone->AddArg(arg);
    }
    return clone;
  }

  OVERRIDE_BASE_CASTABLE
  OVERRIDE_BASE_EQ

  OVERRIDE_UNOP(Deref)
  OVERRIDE_UNOP(Ref);

  Type<> ArrayAccess(const CType& other) const final {
    throw std::runtime_error("Cannot access elements from function pointer");
  }

  Type<> FunctionCall(const std::vector<Type<>>& args) const final {
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
  StructUnionType(LValueNess lvalue, unsigned flags = 0, StorageClass storage = StorageClass::Auto) :
          CType(lvalue, flags, storage) {

  }

  struct Field {
    Field(std::string name, size_t size, const Type<>& contained) :
            name(std::move(name)),
            size(size),
            type(contained->Clone()) {

    }

    Field(std::string name, const Type<>& contained) :
            name(std::move(name)),
            size(0),
            type(contained->Clone()) {

    }

    const std::string name;
    const size_t size = 0;  // 0 means default size
    Type<> type;
  };

  void AddField(const std::string& name, size_t size, const Type<>& contained) {
    fields.push_back(Field(name, size, contained));
  }

  void AddField(const std::string& name, const Type<>& contained) {
    fields.push_back(Field(name, contained));
  }

  Type<> MemberAccess(const std::string& member) const final;

  Type<> Clone() const final {
    auto clone = MakeType<StructUnionType<t>>(lvalue, qualifiers, storage);
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
  StructType(LValueNess lvalue, unsigned flags = 0, StorageClass storage = StorageClass::Auto) :
          StructUnionType(lvalue, flags, storage) {

  }

  std::string ToString() const final {
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
  UnionType(LValueNess lvalue, unsigned flags = 0, StorageClass storage = StorageClass::Auto) :
          StructUnionType(lvalue, flags, storage) {

  }

  std::string ToString() const final {
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
