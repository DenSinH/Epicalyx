#pragma once

#include "Default.h"
#include "Packs.h"


namespace epi::type {

using value_type_pack = cotyl::pack<
  i8, u8, 
  i16, u16,
  i32, u32, 
  i64, u64,
  float, double
>;


struct BaseType;
struct VoidType;
template<typename T>
requires (cotyl::pack_contains_v<T, value_type_pack>)
struct ValueType;
using BoolType = ValueType<i32>;
struct AnyPointerType;
struct PointerType;
struct FunctionType;
struct StructType;
struct UnionType;

struct AnyType;

}