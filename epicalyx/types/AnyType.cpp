#include "AnyType.h"
#include "Decltype.h"
#include "Log.h"


namespace epi::cotyl {

template<> type::detail::any_type_t::~Variant() = default;

}

namespace epi::type {
  
AnyType AnyType::Ref() const {
  // only for lvalues
  // not an lvalue after
  if ((*this)->lvalue == LValue::None) {
    throw std::runtime_error("Cannot get reference to non-lvalue expression");
  }
  // todo: handle function types
  return PointerType{std::make_shared<AnyType>(*this), LValue::None};
//   throw std::runtime_error("not reimplemented");
}

// type.Cast(other) = (type)(other)
// Cast "other" to "this" type
AnyType AnyType::Cast(const AnyType& other, bool check_flags) const {
  if (!TypeEquals(other)) {
    if (other.holds_alternative<StructType>() || other.holds_alternative<UnionType>()) {
      throw std::runtime_error("Bad cast");
    }
  }
  if (check_flags) {
    auto flagdiff = other->qualifiers & ~(*this)->qualifiers;
    if (flagdiff & Qualifier::Const) {
      Log::Warn("Casting drops 'const' qualifier");
    }
    if (flagdiff & Qualifier::Volatile) {
      Log::Warn("Casting drops 'volatile' qualifier");
    }
  }

  AnyType result = *this;
  result.visit<void>(
    [&](auto& dest) {
      if constexpr(cotyl::is_instantiation_of_v<ValueType, decltype_t(dest)>) {
        other.visit<void>(
          [&](const auto& src) {
          if constexpr(cotyl::is_instantiation_of_v<ValueType, decltype_t(src)>) {
              dest.value = src.value;
            }
          }
        );
      }
    }
  );
  result->lvalue = LValue::None;
  return result;
}

AnyType AnyType::CommonType(const AnyType& other) const {
  if (!TypeEquals(other)) {
    return (*this)->CommonTypeImpl(other);
  }
  AnyType result = *this;
  result->ForgetConstInfo();
  result->lvalue = LValue::None;
  return result;
}

bool AnyType::TypeEquals(const AnyType& other) const {
  if (index() != other.index()) {
    return false;
  }

  return visit<bool>(
    [&](const StructType& strct) -> bool {
      return strct.TypeEqualImpl(other.get<StructType>());
    },
    [&](const UnionType& strct) -> bool {
      return strct.TypeEqualImpl(other.get<UnionType>());
    },
    [&](const FunctionType& func) -> bool {
      return func.TypeEqualImpl(other.get<FunctionType>());
    },
    [&](const PointerType& ptr) -> bool {
      return ptr.contained->TypeEquals(*other.get<PointerType>().contained);
    },
    // true by default for value / void types
    [&](const auto&) -> bool { return true; }
  );
}

STRINGIFY_METHOD(AnyType) {
  return value.visit<std::string>([](const auto& dir) -> std::string { 
    return dir.ToString(); 
  });
}

}