#pragma once

#include "Default.h"
#include "Is.h"

#include <memory>

namespace epi::calyx {

using var_index_t = u64;
using pointer_t = u64;

template<typename T>
struct is_calyx_type {
  static constexpr bool value = epi::cotyl::is_in_v<T, i32, u32, i64, u64, float, double>;
};
template<typename T>
constexpr bool is_calyx_type_v = is_calyx_type<T>::value;

// "C variable"
struct CVar {
  var_index_t idx;
  u64 size;
};

struct IROp {

};

using pIROp = std::unique_ptr<IROp>;

enum class Binop {
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  Lsl,
  Lsr,
  Asr,
  BinAnd,
  BinOr,
  BinXor,
};

enum class Unop {
  Neg,
  BinNot
};

struct IRBinop : public IROp {
  var_index_t left_idx;
  Binop op;
  var_index_t right_idx;
};

template<typename T>
struct IRBinopImm : public IROp {
  static_assert(is_calyx_type_v<T>);

  var_index_t left_idx;
  Binop op;
  T value;
};

struct IRUnop : public IROp {
  Binop op;
  var_index_t right_idx;
};

template<typename T>
struct IRLoadCVar : public IROp {
  static_assert(is_calyx_type_v<T>);

  var_index_t c_idx;
  i32 offset;  // struct fields
};

template<typename T>
struct IRStoreCVar : public IROp {
  static_assert(is_calyx_type_v<T>);

  var_index_t c_idx;
  i32 offset;  // struct fields
};

struct IRAllocateCVar : public IROp {
  var_index_t c_idx;
  u64 size;
};

struct IRDeallocateCVar : public IROp {
  var_index_t c_idx;
  u64 size;
};

template<typename T>
struct IRLoadFrom : public IROp {
  static_assert(is_calyx_type_v<T>);

  var_index_t idx;
  i32 offset;
};

template<typename T>
struct IRStoreTo : public IROp {
  static_assert(is_calyx_type_v<T>);

  var_index_t idx;
  i32 offset;
};

struct Return : public IROp {
  var_index_t idx;
};

}