#include "Helpers.h"
#include "types/Types.h"
#include "types/AnyType.h"
#include "Exceptions.h"
#include "Decltype.h"


namespace epi::detail {

using namespace type;

calyx::global_t GetGlobalValue(const AnyType& type) {
  return type.visit<calyx::global_t>(
    [](const StructType&) -> calyx::global_t  {
      throw cotyl::UnimplementedException("global struct");
    },
    [](const UnionType&) -> calyx::global_t {
      throw cotyl::UnimplementedException("global union");
    },
    [](const PointerType& ptr) -> calyx::global_t {
      if (ptr.size == 0) {
        return calyx::Pointer{0};
      }
      else {
        throw cotyl::UnimplementedException("global array");
      }
    },
    [](const FunctionType& func) -> calyx::global_t { 
      return calyx::Pointer{0}; 
    },
    [](const VoidType&) -> calyx::global_t { 
      throw std::runtime_error("Incomplete global type"); 
    },
    [](const auto& value) -> calyx::global_t {
      using value_t = decltype_t(value);
      static_assert(cotyl::is_instantiation_of_v<ValueType, value_t>);
      return (typename value_t::type_t){0};
    }
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
    [](const auto& value) -> std::pair<calyx::Local::Type, u64> {
      using value_t = decltype_t(value);
      static_assert(cotyl::is_instantiation_of_v<ValueType, value_t>);
      return {calyx_loc_type_v<typename value_t::type_t>, 0};
    }
  );
}

}