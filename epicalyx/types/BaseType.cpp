#include "BaseType.h"
#include "Format.h"
#include "Exceptions.h"
#include "AnyType.h"


namespace epi::type {

[[noreturn]] static void InvalidOperands(const BaseType* ths, const std::string& op, const AnyType& other) {
  throw cotyl::FormatExceptStr(
    "Invalid operands for %s: %s and %s",
    op, *ths, other
  );
}

[[noreturn]] static void InvalidOperand(const BaseType* ths, const std::string& op) {
  throw cotyl::FormatExceptStr("Invalid operand for %s: %s", op, *ths);
}


AnyType BaseType::Add(const AnyType& other) const { InvalidOperands(this, "+", other); }
AnyType BaseType::Sub(const AnyType& other) const { InvalidOperands(this, "-", other); }
AnyType BaseType::Mul(const AnyType& other) const { InvalidOperands(this, "*", other); }
AnyType BaseType::Div(const AnyType& other) const { InvalidOperands(this, "/", other); }
AnyType BaseType::Mod(const AnyType& other) const { InvalidOperands(this, "%", other); }
AnyType BaseType::Xor(const AnyType& other) const { InvalidOperands(this, "^", other); }
AnyType BaseType::BinAnd(const AnyType& other) const { InvalidOperands(this, "&", other); }
AnyType BaseType::BinOr(const AnyType& other) const { InvalidOperands(this, "|", other); }

AnyType BaseType::LogAnd(const AnyType& other) const { InvalidOperands(this, "&&", other); }
AnyType BaseType::LogOr(const AnyType& other) const { InvalidOperands(this, "||", other); }
AnyType BaseType::LogNot() const { InvalidOperand(this, "~"); }

AnyType BaseType::Lt(const AnyType& other) const { InvalidOperands(this, "<", other); }
AnyType BaseType::Eq(const AnyType& other) const { InvalidOperands(this, "==", other); }
AnyType BaseType::LShift(const AnyType& other) const { InvalidOperands(this, "<<", other); }
AnyType BaseType::RShift(const AnyType& other) const { InvalidOperands(this, ">>", other); }

// combinations of above functions
AnyType BaseType::Gt(const AnyType& other) const {
  return Le(other)->LogNot();  // !(<=)
}

AnyType BaseType::Le(const AnyType& other) const {
  auto lt = this->Lt(other);
  auto eq = this->Eq(other);
  return lt->LogOr(eq);  // < || ==
}

AnyType BaseType::Ge(const AnyType& other) const {
  auto gt = this->Gt(other);
  auto eq = this->Eq(other);
  return gt->LogOr(eq);  // > || ==
}

AnyType BaseType::Neq(const AnyType& other) const {
  auto eq = this->Eq(other);
  return eq->LogNot();   // !( == )
}


AnyType BaseType::MemberAccess(const cotyl::CString& member) const { InvalidOperand(this, "."); }
AnyType BaseType::FunctionCall(const cotyl::vector<AnyType>& args) const { InvalidOperand(this, "()"); }

AnyType BaseType::Deref() const { InvalidOperand(this, "*"); };
AnyType BaseType::ArrayAccess(const AnyType& other) const { InvalidOperand(this, "[]"); };

// AnyType Ref() const;  // we want a different default here
AnyType BaseType::Neg() const { InvalidOperand(this, "-"); }
AnyType BaseType::Pos() const { InvalidOperand(this, "+"); }
AnyType BaseType::BinNot() const { InvalidOperand(this, "~"); }
AnyType BaseType::Incr() const { InvalidOperand(this, "++"); }
AnyType BaseType::Decr() const { InvalidOperand(this, "--"); }

AnyType BaseType::Ref() const {
  // only for lvalues
  // not an lvalue after
  if (lvalue == LValueNess::None) {
    throw std::runtime_error("Cannot get reference to non-lvalue expression");
  }
  // todo: handle function types
  throw std::runtime_error("not reimplemented");
}

u64 BaseType::Alignof() const {
  u64 size = Sizeof();
  return size < 3 ? size : 4;
}

}