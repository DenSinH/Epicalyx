#pragma once

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <stdexcept>

#include "Default.h"
#include "Format.h"

namespace epi {

struct InitializerList;
struct Parser;
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

struct TypeVisitor {
  virtual void Visit(const VoidType& type) = 0;
  virtual void Visit(const ValueType<i8>& type) = 0;
  virtual void Visit(const ValueType<u8>& type) = 0;
  virtual void Visit(const ValueType<i16>& type) = 0;
  virtual void Visit(const ValueType<u16>& type) = 0;
  virtual void Visit(const ValueType<i32>& type) = 0;
  virtual void Visit(const ValueType<u32>& type) = 0;
  virtual void Visit(const ValueType<i64>& type) = 0;
  virtual void Visit(const ValueType<u64>& type) = 0;
  virtual void Visit(const ValueType<float>& type) = 0;
  virtual void Visit(const ValueType<double>& type) = 0;
  virtual void Visit(const PointerType& type) = 0;
  virtual void Visit(const ArrayType& type) = 0;
  virtual void Visit(const FunctionType& type) = 0;
  virtual void Visit(const StructType& type) = 0;
  virtual void Visit(const UnionType& type) = 0;
};

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
#define VIRTUAL_CASTABLE(_, type) virtual pType<> CastTo(const type& other) const { throw std::runtime_error("Bad cast"); }
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
  virtual bool HasTruthiness() const { return false; }
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

  VIRTUAL_BINOP(CommonType)
  virtual u64 Sizeof() const { throw std::runtime_error("Size of incomplete type"); }
  u64 Alignof() const {
    u64 size = Sizeof();
    return size < 3 ? size : 4;
  }
  // type.Cast(other) = (type)(other)
  pType<> Cast(const CType& other) const;
  virtual pType<> DoCast(const CType& other) const { throw std::runtime_error("Bad cast"); }  // checking whether types are castable
  virtual bool EqualType(const CType& other) const { return false; }  // for checking complete equality
  virtual bool IsFunction() const { return false; }
  virtual bool IsIntegral() const { return false; }
  virtual i64 ConstIntVal() const { throw std::runtime_error("Type is not an integer value: " + to_string()); }
  virtual pType<> Clone() const = 0;
  virtual void Visit(TypeVisitor& v) const = 0;

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

}