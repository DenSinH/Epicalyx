#pragma once

#include "Types.h"
#include "Stringify.h"
#include "Vector.h"

#include <memory>

namespace epi::type {

namespace detail {

using any_type_t = cotyl::Variant<BaseType,
  VoidType,
  PointerType,
  ArrayType,
  FunctionType,
  StructType,
  UnionType,
  ValueType<i8>,
  ValueType<u8>,
  ValueType<i16>,
  ValueType<u16>,
  ValueType<i32>,
  ValueType<u32>,
  ValueType<i64>,
  ValueType<u64>,
  ValueType<float>,
  ValueType<double>
>;

}

struct AnyType : detail::any_type_t {
  using detail::any_type_t::any_type_t;

  // type.Cast(other) = (type)(other)
  // Cast "other" to "this" type
  AnyType Ref() const;
  AnyType Cast(const AnyType& other, bool check_flags = true) const;
  AnyType CommonType(const AnyType& other) const;
  bool TypeEquals(const AnyType& other) const;
};

STRINGIFY_METHOD(AnyType);

}