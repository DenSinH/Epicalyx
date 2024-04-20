#pragma once

#include <string>

#include "Default.h"


namespace epi::type {

struct AnyType;

struct BaseType {

  enum Qualifier : u8 {
    Const = 0x1,
    Restrict = 0x2,
    Volatile = 0x4,
    Atomic = 0x8,
  };

  enum class LValueNess : u8 {
    None = 0,
    LValue,
    Assignable,
  };

  BaseType(LValueNess lvalue, u8 flags = 0) : qualifiers{flags}, lvalue{lvalue} { }

  u8 qualifiers = 0;
  LValueNess lvalue;

  // virtual bool IsConstexpr() const { return false; }  // for optimizing branching
  // virtual bool GetBoolValue() const { throw std::runtime_error("Type cannot be converted to bool"); }
  // virtual bool HasTruthiness() const { return false; }
  AnyType TruthinessAsCType() const;
  bool IsAssignable() const { 
    return lvalue == LValueNess::Assignable && !(qualifiers & Qualifier::Const); 
  }

  virtual std::string ToString() const = 0;

  virtual AnyType Add(const AnyType& other) const;
  virtual AnyType Sub(const AnyType& other) const;
  virtual AnyType Mul(const AnyType& other) const;
  virtual AnyType Div(const AnyType& other) const;
  virtual AnyType Mod(const AnyType& other) const;
  virtual AnyType Xor(const AnyType& other) const;
  virtual AnyType BinAnd(const AnyType& other) const;
  virtual AnyType BinOr(const AnyType& other) const;

  AnyType LogAnd(const AnyType& other) const;
  AnyType LogOr(const AnyType& other) const;
  AnyType LogNot() const;

  virtual AnyType Lt(const AnyType& other) const;
  virtual AnyType Eq(const AnyType& other) const;
  virtual AnyType LShift(const AnyType& other) const;
  virtual AnyType RShift(const AnyType& other) const;

  // combinations of above functions
  AnyType Gt(const AnyType& other) const;
  AnyType Le(const AnyType& other) const;
  AnyType Ge(const AnyType& other) const;
  AnyType Neq(const AnyType& other) const;

  virtual AnyType MemberAccess(const cotyl::CString& member) const;
  virtual AnyType FunctionCall(const std::vector<AnyType>& args) const;

  virtual AnyType Deref() const;
  AnyType ArrayAccess(const AnyType& other) const;

  AnyType Ref() const;  // we want a different default here
  virtual AnyType Neg() const;
  virtual AnyType Pos() const;  // really just sets LValueNess to None
  virtual AnyType BinNot() const;
  virtual AnyType Incr() const;
  virtual AnyType Decr() const;

  virtual u64 Sizeof() const = 0;
  u64 Alignof() const;

  virtual AnyType CommonTypeImpl(const AnyType& other) const = 0;

  // forget info on contained value (for example, adding an int to a pointer)
  virtual void ForgetConstInfo() { }
};

}