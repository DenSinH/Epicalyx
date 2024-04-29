#include "BaseType.h"
#include "Format.h"
#include "Exceptions.h"
#include "AnyType.h"


namespace epi::type {

[[noreturn]] static void InvalidOperands(const BaseType* ths, const std::string& op, const AnyType& other) {
  throw cotyl::FormatExceptStr<TypeError>(
    "Invalid operands for %s: %s and %s",
    op, *ths, other
  );
}

[[noreturn]] static void InvalidOperand(const BaseType* ths, const std::string& op) {
  throw cotyl::FormatExceptStr<TypeError>("Invalid operand for %s: %s", op, *ths);
}


AnyType BaseType::Add(const AnyType& other) const { InvalidOperands(this, "+", other); }
AnyType BaseType::Sub(const AnyType& other) const { InvalidOperands(this, "-", other); }
AnyType BaseType::Mul(const AnyType& other) const { InvalidOperands(this, "*", other); }
AnyType BaseType::Div(const AnyType& other) const { InvalidOperands(this, "/", other); }
AnyType BaseType::Mod(const AnyType& other) const { InvalidOperands(this, "%", other); }
AnyType BaseType::Xor(const AnyType& other) const { InvalidOperands(this, "^", other); }
AnyType BaseType::BinAnd(const AnyType& other) const { InvalidOperands(this, "&", other); }
AnyType BaseType::BinOr(const AnyType& other) const { InvalidOperands(this, "|", other); }


BoolType BaseType::Truthiness() const {
  throw cotyl::FormatExceptStr<TypeError>("%s has no truthiness");
}

BoolType BaseType::LogAnd(const AnyType& other) const { 
  auto truthiness = Truthiness();
  auto rtruthiness = other->Truthiness();

  if (rtruthiness.value.has_value()) {
    std::swap(truthiness, rtruthiness);
  }

  if (truthiness.value.has_value()) {
    if (!truthiness.value.value()) {
      // false, return false
      return truthiness;
    }
    else {
      // true, return rtruthiness
      return rtruthiness;
    }
  }

  // neither has a truth value
  return truthiness;
}

BoolType BaseType::LogOr(const AnyType& other) const { 
  auto truthiness = Truthiness();
  auto rtruthiness = other->Truthiness();

  if (rtruthiness.value.has_value()) {
    std::swap(truthiness, rtruthiness);
  }

  if (truthiness.value.has_value()) {
    if (truthiness.value.value()) {
      // true, return true
      return truthiness;
    }
    else {
      // false, return other
      return rtruthiness;
    }
  }

  // neither has a truth value
  return truthiness;
}

BoolType BaseType::LogNot() const { 
  auto truthiness = Truthiness();
  if (truthiness.value.has_value()) {
    truthiness.value.emplace(truthiness.value.value() ? 0 : 1);
  }
  return truthiness;
}

BoolType BaseType::Lt(const AnyType& other) const { InvalidOperands(this, "comparison", other); }
BoolType BaseType::Eq(const AnyType& other) const { InvalidOperands(this, "comparison", other); }
AnyType BaseType::LShift(const AnyType& other) const { InvalidOperands(this, "<<", other); }
AnyType BaseType::RShift(const AnyType& other) const { InvalidOperands(this, ">>", other); }

// combinations of above functions
BoolType BaseType::Gt(const AnyType& other) const {
  return Le(other).LogNot();  // !(<=)
}

BoolType BaseType::Le(const AnyType& other) const {
  auto lt = this->Lt(other);
  auto eq = this->Eq(other);
  return lt.LogOr(eq);  // < || ==
}

BoolType BaseType::Ge(const AnyType& other) const {
  auto gt = this->Lt(other);
  return gt.LogNot();  // !(<)
}

BoolType BaseType::Neq(const AnyType& other) const {
  auto eq = this->Eq(other);
  return eq.LogNot();   // !( == )
}


AnyType BaseType::MemberAccess(const cotyl::CString& member) const { InvalidOperand(this, "."); }
AnyType BaseType::FunctionCall(const cotyl::vector<AnyType>& args) const { InvalidOperand(this, "()"); }

AnyType BaseType::Deref() const { InvalidOperand(this, "*"); };
AnyType BaseType::ArrayAccess(const AnyType& other) const { 
  return Add(other)->Deref(); 
};

// AnyType Ref() const;  // we want a different default here
AnyType BaseType::Neg() const { InvalidOperand(this, "-"); }
AnyType BaseType::Pos() const { InvalidOperand(this, "+"); }
AnyType BaseType::BinNot() const { InvalidOperand(this, "~"); }

AnyType BaseType::Incr() const { 
  if (lvalue != LValue::Assignable) {
    throw TypeError("Expression is not assignable");
  }
  return Add(ValueType<i32>(1, LValue::None));
}

AnyType BaseType::Decr() const { 
  if (lvalue != LValue::Assignable) {
    throw TypeError("Expression is not assignable");
  }
  return Sub(ValueType<i32>(1, LValue::None));
}

u64 BaseType::Alignof() const {
  u64 size = Sizeof();
  return size < 3 ? size : 4;
}

}