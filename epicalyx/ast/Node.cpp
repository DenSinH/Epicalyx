#include "Node.h"
#include "Decltype.h"


namespace epi::ast {

void ExprNode::VerifySwitchable() const {
  type::AnyType switch_val = type::ValueType<i64>(type::LValue::Assignable);
  switch_val.Cast(type, false);
}

void ExprNode::VerifyTruthiness() const {
  type->Truthiness();
}

bool ExprNode::IsConstexpr() const {
  return type.visit<bool>(
    []<typename T>(const type::ValueType<T>& value) -> bool {
      return value.value.has_value();
    },
    [](const auto& value) -> bool {
      return false;
    }
  );
}

bool ExprNode::ConstBoolVal() const {
  auto truthiness = type->Truthiness();
  if (!truthiness.value.has_value()) {
    throw cotyl::Exception("AST Error", "Expected constant expression");
  }
  return truthiness.value.value() ? true : false;
}

i64 ExprNode::ConstIntVal() const {
  return type.visit<i64>(
    []<typename T>(const type::ValueType<T>& value) -> i64 {
      if (!value.value.has_value()) {
        throw cotyl::Exception("AST Error", "Expected constant expression");
      }
      return (i64)value.value.value();
    },
    [](const auto& value) -> i64 {
      throw cotyl::Exception("AST Error", "Expected constant expression");
    }
  );
}


}