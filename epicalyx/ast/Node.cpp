#include "Node.h"
#include "Decltype.h"


namespace epi::ast {

void ExprNode::VerifySwitchable() const {
  type::AnyType switch_val = type::ValueType<i64>(type::BaseType::LValueNess::Assignable);
  switch_val.Cast(type);
}

void ExprNode::VerifyTruthiness() const {
  type::AnyType bool_val = type::MakeBool(type::BaseType::LValueNess::Assignable);
  bool_val.Cast(type);
}

bool ExprNode::IsConstexpr() const {
  return type.visit<bool>(
    [](const auto& value) -> bool {
      using value_t = decltype_t(value);
      if constexpr(cotyl::is_instantiation_of_v<type::ValueType, value_t>) {
        return value.value.has_value();
      }
      return false;
    }
  );
}

bool ExprNode::ConstBoolVal() const {
  return type.visit<bool>(
    [](const auto& value) -> bool {
      using value_t = decltype_t(value);
      if constexpr(cotyl::is_instantiation_of_v<type::ValueType, value_t>) {
        if (!value.value.has_value()) {
          throw std::runtime_error("Expected constant expression");
        }
        return value.value.value() != 0;
      }
      else {
        throw std::runtime_error("Expected constant expression");
      }
    }
  );
}

i64 ExprNode::ConstIntVal() const {
  return type.visit<i64>(
    [](const auto& value) -> i64 {
      using value_t = decltype_t(value);
      if constexpr(cotyl::is_instantiation_of_v<type::ValueType, value_t>) {
        if (!value.value.has_value()) {
          throw std::runtime_error("Expected constant expression");
        }
        return (i64)value.value.value();
      }
      else {
        throw std::runtime_error("Expected constant expression");
      }
    }
  );
}


}