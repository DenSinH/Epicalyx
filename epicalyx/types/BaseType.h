#pragma once

#include <string>

#include "Default.h"
#include "TypeFwd.h"
#include "Exceptions.h"


namespace epi::type {

struct TypeError : cotyl::Exception {
  TypeError(std::string&& message) : 
      Exception("Type Error", std::move(message)) { }
};

enum Qualifier : u8 {
  Const = 0x1,
  Restrict = 0x2,
  Volatile = 0x4,
  Atomic = 0x8,
};

enum class LValue : u8 {
  None = 0,
  LValue,
  Assignable,
};

struct BaseType {

  BaseType(LValue lvalue, u8 flags = 0) : qualifiers{flags}, lvalue{lvalue} { }

  u8 qualifiers = 0;
  LValue lvalue;

  bool IsAssignable() const { 
    return lvalue == LValue::Assignable && !(qualifiers & Qualifier::Const); 
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

  virtual BoolType Truthiness() const;
  BoolType LogAnd(const AnyType& other) const;
  BoolType LogOr(const AnyType& other) const;
  BoolType LogNot() const;

  virtual BoolType Lt(const AnyType& other) const;
  virtual BoolType Eq(const AnyType& other) const;
  virtual AnyType LShift(const AnyType& other) const;
  virtual AnyType RShift(const AnyType& other) const;

  // combinations of above functions
  BoolType Gt(const AnyType& other) const;
  BoolType Le(const AnyType& other) const;
  BoolType Ge(const AnyType& other) const;
  BoolType Neq(const AnyType& other) const;

  virtual AnyType MemberAccess(const cotyl::CString& member) const;
  virtual AnyType FunctionCall(const cotyl::vector<AnyType>& args) const;

  virtual AnyType Deref() const;
  AnyType ArrayAccess(const AnyType& other) const;

  virtual AnyType Neg() const;
  virtual AnyType Pos() const;  // really just sets LValue to None
  virtual AnyType BinNot() const;
  AnyType Incr() const;
  AnyType Decr() const;

  virtual u64 Sizeof() const = 0;
  virtual u32 Alignof() const;

  virtual AnyType CommonTypeImpl(const AnyType& other) const = 0;

  // forget info on contained value (for example, adding an int to a pointer)
  virtual void ForgetConstInfo() const { }
};

}