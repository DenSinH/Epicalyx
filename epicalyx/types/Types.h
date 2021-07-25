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
#define VIRTUAL_BINOP(handler) public: virtual pType<> handler(const CType& other) const { throw std::runtime_error("Unimplemented binop handler"); }; protected: VIRTUAL_BINOP_HANDLERS(R ## handler) public:
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

  CType(LValueNess lvalue, u32 flags = 0) :
          lvalue(lvalue),
          qualifiers(flags) {

  }

  u32 qualifiers = 0;
  LValueNess lvalue;

  static pType<ValueType<i8>> ConstOne();
  virtual bool IsConstexpr() const { return false; }  // for optimizing branching
  virtual bool GetBoolValue() const { throw std::runtime_error("Type cannot be converted to bool"); }
  pType<ValueType<i32>> TruthinessAsCType() const;
  bool IsAssignable() const { return lvalue == LValueNess::Assignable && !(qualifiers & Qualifier::Const); }

  virtual std::string to_string() const = 0;

  VIRTUAL_BINOP(Add)
  VIRTUAL_BINOP(Sub)
  VIRTUAL_BINOP(Mul)
  VIRTUAL_BINOP(Div)
  VIRTUAL_BINOP(Mod)
  VIRTUAL_BINOP(Xor)
  VIRTUAL_BINOP(BinAnd)
  VIRTUAL_BINOP(BinOr)

  pType<ValueType<i32>> LogAnd(const CType& other) const;
  pType<ValueType<i32>> LogOr(const CType& other) const;
  pType<ValueType<i32>> LogNot() const;

  pType<> LLogAnd(const CType& other) const;
  pType<> LLogOr(const CType& other) const;
  pType<> LLogNot() const;

  VIRTUAL_BINOP(Lt)
  VIRTUAL_BINOP(Eq)
  VIRTUAL_BINOP(LShift)
  VIRTUAL_BINOP(RShift)

  // combinations of above functions
  pType<> Gt(const CType& other) const;
  pType<> Le(const CType& other) const;
  pType<> Ge(const CType& other) const;
  pType<> Neq(const CType& other) const;

  virtual pType<> MemberAccess(const std::string& member) const {
    throw std::runtime_error("Cannot access member of non-struct/union type");
  }

  virtual pType<> FunctionCall(const std::vector<pType<const CType>>& args) const {
    throw std::runtime_error("Expression is not a function");
  }

  VIRTUAL_UNOP(Deref)
  pType<> ArrayAccess(const CType& other) const;

  UNOP_HANDLER(Ref);  // we want a different default here
  VIRTUAL_UNOP(Neg)
  VIRTUAL_UNOP(Pos)  // really just sets LValueNess to None
  VIRTUAL_UNOP(BinNot)
  UNOP_HANDLER(Incr);
  UNOP_HANDLER(Decr);

  virtual u64 Sizeof() const { throw std::runtime_error("Size of incomplete type"); }
  virtual u64 Alignof() const { throw std::runtime_error("Align of incomplete type"); }
  virtual bool CastableType(const CType& other) const { return false; }  // checking whether types are castable
  virtual bool EqualType(const CType& other) const { return false; }  // for checking complete equality
  virtual bool IsFunction() const { return false; }
  virtual i64 ConstIntVal() const { throw std::runtime_error("Type is not an integer value: " + to_string()); }
  virtual pType<> Clone() const = 0;

protected:
  virtual bool IsComplete() const { return false; }

  virtual void ForgetConstInfo() { }  // forget info on contained value (for example, adding an int to a pointer)

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

  OVERRIDE_BASE_EQ

  bool IsComplete() const final { return false; }
  bool CastableType(const CType& other) const final {  return true; }  // any type can be cast to void

  std::string to_string() const final { return "void"; };

protected:
  bool EqualTypeImpl(const VoidType& other) const final { return true; }
  pType<> Clone() const final { return MakeType<VoidType>(qualifiers); }
};


template<typename T>
struct ValueType : public CType {
  explicit ValueType(LValueNess lvalue, u32 flags = 0) :
          CType(lvalue, flags),
          value() {

  }

  explicit ValueType(T value, LValueNess lvalue, u32 flags = 0) :
          CType(lvalue, flags),
          value(value) {

  }

  std::optional<T> value;

  OVERRIDE_BASE_CASTABLE
  OVERRIDE_BASE_EQ

  constexpr T Get() const { return value.value(); }
  constexpr bool HasValue() const { return value.has_value(); }
  bool IsConstexpr() const final { return HasValue(); }

  bool GetBoolValue() const final {
    if (HasValue()) {
      return Get() != 0;
    }
    throw std::runtime_error("Bool value requested from non-constant Get");
  }

  std::string to_string() const final {
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

  u64 Sizeof() const final { return sizeof(T); }
  u64 Alignof() const final { return sizeof(T); }  // todo:

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

  bool EqualTypeImpl(const ValueType<T>& other) const final { return true; }
};


struct PointerType : public CType {
  PointerType(const pType<>& contained, LValueNess lvalue, u32 flags = 0) :
      CType(lvalue, flags),
      contained(contained ? contained->Clone() : nullptr) {

  }

  pType<> contained;

  std::string to_string() const override { return cotyl::FormatStr("(%s)*", contained); }
  bool CastableType(const CType& other) const override { return other.CastableType(*this); }
  bool EqualType(const CType& other) const override { return other.EqualTypeImpl(*this); }

  OVERRIDE_BINOP(Add)
  OVERRIDE_BINOP(Sub)
  pType<> Deref() const override;

  u64 Sizeof() const final { return sizeof(u64); }
  u64 Alignof() const final { return sizeof(u64); }  // todo:

  pType<> Clone() const override {
    return MakeType<PointerType>(contained ? contained->Clone() : nullptr, lvalue, qualifiers);
  }

private:
  void ForgetConstInfo() override { if (contained) contained->ForgetConstInfo(); }

  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RAdd)
  // we cannot do `int - ptr_type`, so we must not override this
  OVERRIDE_BINOP_HANDLER(RSub, PointerType);
  OVERRIDE_BINOP_HANDLER(RLt, PointerType);
  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, RLt)
  OVERRIDE_BINOP_HANDLER(REq, PointerType);
  INTEGRAL_TYPE_SIGNATURES(OVERRIDE_BINOP_HANDLER_NOIMPL, REq)

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
    return cotyl::FormatStr("(%s)[%s]", contained, size);
  }

  pType<> Clone() const override {
    return MakeType<ArrayType>(contained, size, qualifiers);
  }
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
    std::string to_string() const;
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

  std::string to_string() const final;
  bool IsFunction() const final { return true; }
  pType<> Clone() const final;

  OVERRIDE_UNOP(Deref)

  pType<> FunctionCall(const std::vector<pType<const CType>>& args) const final;

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

  std::string to_string() const final;
  pType<> MemberAccess(const std::string& member) const final;

  u64 Sizeof() const final;
  u64 Alignof() const final { return Sizeof(); } // todo:

protected:
  virtual std::string BaseString() const = 0;
  bool _CastableTypeImpl(const StructUnionType& other) const {
    return _EqualTypeImpl(other);
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

protected:
  std::string BaseString() const final { return "struct"; }

private:
  bool CastableTypeImpl(const StructType& other) const final { return _CastableTypeImpl(other); }
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

protected:
  std::string BaseString() const final { return "struct"; }

private:
  bool CastableTypeImpl(const UnionType& other) const final { return _CastableTypeImpl(other); }
  bool EqualTypeImpl(const UnionType& other) const final { return _EqualTypeImpl(other); }
};

}
