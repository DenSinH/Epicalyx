#include "AnyType.h"

#include <memory>  // for __shared_ptr_access, make_shared
#include <string>  // for string

#include "Log.h"   // for Warn


namespace epi::cotyl {

template<> type::detail::any_type_t::~Variant() = default;

}

namespace epi::type {
  
AnyType AnyType::Ref() const {
  // only for lvalues
  // not an lvalue after
  if ((*this)->lvalue == LValue::None) {
    throw TypeError("Cannot get reference to non-lvalue expression");
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
      throw TypeError("Bad cast");
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
  result->ForgetConstInfo();
  result.visit<void>(
    [&]<typename T>(ValueType<T>& dest) {
      other.visit<void>(
        [&]<typename R>(const ValueType<R>& src) {
          if (src.value.has_value()) {
            dest.value = src.value.value();
          }
        },
        [](const auto&) { }
      );
    },
    [](const auto&) { }
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
    [&](const ArrayType& arr) -> bool {
      const auto& other_arr = other.get<ArrayType>();
      if (arr.size && other_arr.size && arr.size != other_arr.size) {
        return false;
      }
      return arr.contained->TypeEquals(*other_arr.contained);
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