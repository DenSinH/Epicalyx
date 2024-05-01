#pragma once

#include "Default.h"


namespace epi {

namespace calyx {

struct Function;

}

using var_index_t = u32;
using loc_index_t = var_index_t;
using block_label_t = u32;
using func_pos_t = std::pair<block_label_t, int>;

struct program_pos_t {
  const calyx::Function* func;
  func_pos_t pos;
};

namespace calyx {

struct Pointer;
struct Aggregate;

using calyx_small_types = cotyl::pack<i8, u8, i16, u16>;
using calyx_integral_types = cotyl::pack<i32, u32, i64, u64>;
using calyx_arithmetic_types = cotyl::flatten_pack<calyx_integral_types, float, double>;
using calyx_types = cotyl::flatten_pack<calyx_arithmetic_types, calyx::Pointer>;
using calyx_return_types = cotyl::flatten_pack<calyx_types, void>;
using calyx_memory_types = cotyl::flatten_pack<calyx_types, calyx_small_types>;

template<typename T>
constexpr bool is_calyx_small_type_v = cotyl::pack_contains_v<T, calyx_small_types>;
template<typename T>
constexpr bool is_calyx_integral_type_v = cotyl::pack_contains_v<T, calyx_integral_types>;
template<typename T>
constexpr bool is_calyx_arithmetic_type_v = cotyl::pack_contains_v<T, calyx_arithmetic_types>;
template<typename T>
constexpr bool is_calyx_type_v = cotyl::pack_contains_v<T, calyx_types>;

template<typename T> struct calyx_upcast { using type = T; };
template<> struct calyx_upcast<i8> { using type = i32; };
template<> struct calyx_upcast<u8> { using type = u32; };
template<> struct calyx_upcast<i16> { using type = i32; };
template<> struct calyx_upcast<u16> { using type = u32; };
template<typename T>
using calyx_upcast_t = typename calyx_upcast<T>::type;

#define calyx_op_type(op) typename decltype_t(op)

template<typename T>
struct Scalar;

template<typename T>
requires (is_calyx_type_v<T>)
struct Operand;
struct Local;

// IR var idx and Argument
using arg_list_t = cotyl::vector<std::pair<var_index_t, Local>>;

struct ArgData;

struct LabelOffset;
struct Global;

struct Directive;

struct Expr;
struct Branch;
template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct Store;

struct NoOp;

template<typename To, typename From>
requires (
  is_calyx_type_v<From> && 
  (is_calyx_type_v<To> || is_calyx_small_type_v<To>)
)
struct Cast;

template<typename T>
requires (is_calyx_arithmetic_type_v<T>)
struct Binop;

template<typename T>
requires (is_calyx_integral_type_v<T>)
struct Shift;

template<typename T>
requires (is_calyx_type_v<T>)
struct Compare;

struct UnconditionalBranch;

template<typename T>
requires (is_calyx_type_v<T>)
struct BranchCompare;

template<typename T>
requires (is_calyx_integral_type_v<T>)
struct AddToPointer;

template<typename T>
requires (is_calyx_type_v<T>)
struct Imm;

template<typename T>
requires (is_calyx_arithmetic_type_v<T>)
struct Unop;

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct LoadLocal;

struct LoadLocalAddr;

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct StoreLocal;

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct LoadGlobal;

struct LoadGlobalAddr;

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct StoreGlobal;


template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct LoadFromPointer;

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct StoreToPointer;

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_return_types>)
struct Return;

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_return_types>)
struct Call;

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_return_types>)
struct CallLabel;

struct Select;

struct AnyExpr;
struct AnyDirective;

struct BasicBlock;
struct Function;
struct Program;

}

}