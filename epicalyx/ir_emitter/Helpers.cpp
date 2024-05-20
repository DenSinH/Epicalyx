#include "Helpers.h"
#include "types/Types.h"
#include "types/AnyType.h"
#include "Exceptions.h"
#include "Decltype.h"


namespace epi::detail {

using namespace type;

calyx::Global GetGlobalValue(const AnyType& type) {
  return type.visit<calyx::Global>(
    [](const VoidType&) -> calyx::Global { 
      throw type::TypeError("Incomplete global type"); 
    },
    [](const StructType& strct) -> calyx::Global  {
      return calyx::AggregateData{strct.Sizeof(), strct.Alignof()};
    },
    [](const UnionType& strct) -> calyx::Global {
      return calyx::AggregateData{strct.Sizeof(), strct.Alignof()};
    },
    [](const PointerType& ptr) -> calyx::Global {
      return calyx::Pointer{0};
    },
    [](const type::ArrayType& strct) -> calyx::Global {
      return calyx::AggregateData{strct.Sizeof(), strct.Alignof()};
    },
    [](const FunctionType& func) -> calyx::Global { 
      return calyx::Pointer{0}; 
    },
    []<typename T>(const ValueType<T>& value) -> calyx::Global {
      return calyx::Scalar<T>{0};
    },
    swl::exhaustive
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


calyx::Local MakeLocal(loc_index_t loc_idx, const type::AnyType& type) {
  return type.visit<calyx::Local>(
    [](const VoidType&) -> calyx::Local {
      throw type::TypeError("Incomplete local type");
    },
    [&](const StructType& strct) -> calyx::Local {
      return calyx::Local::Aggregate(loc_idx, calyx::Aggregate{strct.Sizeof(), strct.Alignof()}); 
    },
    [&](const UnionType& strct) -> calyx::Local { 
      return calyx::Local::Aggregate(loc_idx, calyx::Aggregate{strct.Sizeof(), strct.Alignof()}); 
    },
    [&](const PointerType& ptr) -> calyx::Local {
      return calyx::Local::Pointer(loc_idx, ptr.Stride()); 
    },
    [&](const ArrayType& arr) -> calyx::Local {
      if (arr.size) {
        return calyx::Local::Aggregate(loc_idx, calyx::Aggregate{arr.Sizeof(), arr.Alignof()});
      }
      else {
        return calyx::Local::Pointer(loc_idx, arr.Stride()); 
      }         
    },
    [&](const FunctionType&) -> calyx::Local { 
      return calyx::Local::Pointer(loc_idx, 0);
    },
    [&]<typename T>(const ValueType<T>& value) -> calyx::Local {
      return {calyx_loc_type_v<T>, loc_idx};
    },
    swl::exhaustive
  );
}

}