#include "Helpers.h"
#include "types/Types.h"
#include "types/AnyType.h"
#include "Exceptions.h"
#include "Decltype.h"


namespace epi::detail {

using namespace type;

calyx::Global GetGlobalValue(const AnyType& type) {
  return type.visit<calyx::Global>(
    [](const StructType&) -> calyx::Global  {
      throw cotyl::UnimplementedException("global struct");
    },
    [](const UnionType&) -> calyx::Global {
      throw cotyl::UnimplementedException("global union");
    },
    [](const PointerType& ptr) -> calyx::Global {
      if (ptr.size == 0) {
        return calyx::Pointer{0};
      }
      else {
        throw cotyl::UnimplementedException("global array");
      }
    },
    [](const FunctionType& func) -> calyx::Global { 
      return calyx::Pointer{0}; 
    },
    [](const VoidType&) -> calyx::Global { 
      throw std::runtime_error("Incomplete global type"); 
    },
    []<typename T>(const ValueType<T>& value) -> calyx::Global {
      return calyx::Scalar<T>{0};
    },
    // exhaustive variant access
    [](const auto& invalid) { static_assert(!sizeof(invalid)); }
  );
}

template<typename T> struct calyx_loc_type;
template<> struct calyx_loc_type<i8> { static constexpr auto value = calyx::Local::Type::I8; };
template<> struct calyx_loc_type<u8> { static constexpr auto value = calyx::Local::Type::U8; };
template<> struct calyx_loc_type<i16> { static constexpr auto value = calyx::Local::Type::I16; };
template<> struct calyx_loc_type<u16> { static constexpr auto value = calyx::Local::Type::U16; };
template<> struct calyx_loc_type<i32> { static constexpr auto value = calyx::Local::Type::I32; };
template<> struct calyx_loc_type<u32> { static constexpr auto value = calyx::Local::Type::U32; };
template<> struct calyx_loc_type<i64> { static constexpr auto value = calyx::Local::Type::I64; };
template<> struct calyx_loc_type<u64> { static constexpr auto value = calyx::Local::Type::U64; };
template<> struct calyx_loc_type<float> { static constexpr auto value = calyx::Local::Type::Float; };
template<> struct calyx_loc_type<double> { static constexpr auto value = calyx::Local::Type::Double; };
template<typename T>
constexpr auto calyx_loc_type_v = calyx_loc_type<T>::value;


std::pair<calyx::Local::Type, u64> GetLocalType(const type::AnyType& type) {
  return type.visit<std::pair<calyx::Local::Type, u64>>(
    [](const VoidType&) -> std::pair<calyx::Local::Type, u64> {
      throw std::runtime_error("Incomplete local type");
    },
    [](const StructType& strct) -> std::pair<calyx::Local::Type, u64> {
      return {calyx::Local::Type::Struct, strct.Sizeof()}; 
    },
    [](const UnionType& strct) -> std::pair<calyx::Local::Type, u64> { 
      return {calyx::Local::Type::Struct, strct.Sizeof()}; 
    },
    [](const PointerType& ptr) -> std::pair<calyx::Local::Type, u64> { 
      return {calyx::Local::Type::Pointer, ptr.Stride()}; 
    },
    [](const FunctionType&) -> std::pair<calyx::Local::Type, u64> { 
      return {calyx::Local::Type::Pointer, 0}; 
    },
    []<typename T>(const ValueType<T>& value) -> std::pair<calyx::Local::Type, u64> {
      return {calyx_loc_type_v<T>, 0};
    },
    // exhaustive variant access
    [](const auto& invalid) { static_assert(!sizeof(invalid)); }
  );
}

}