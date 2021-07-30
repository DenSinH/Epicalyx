#pragma once

#include "Default.h"
#include "Is.h"

#include <memory>
#include <string>


namespace epi::calyx {

using var_index_t = u64;
using pointer_t = u64;

template<typename T>
struct is_calyx_type {
  static constexpr bool value = epi::cotyl::is_in_v<T, i32, u32, i64, u64, float, double>;
};
template<typename T>
constexpr bool is_calyx_type_v = is_calyx_type<T>::value;

struct CVar {
  enum class Location {
    Stack,  // if address is taken
    Register,
    Either,
  };

  var_index_t idx;
  Location loc;
  u64 size;
};

struct IROp {
  enum class Class {
    Expression,  // includes loads
    Store,
    Stack,
    Branch,
  };

  IROp(Class cls) :
      cls(cls) {

  }

  virtual ~IROp() = default;

  Class cls;

  virtual std::string ToString() const = 0;
};

struct IRExpr : public IROp {
  IRExpr(var_index_t idx) :
    IROp(Class::Expression),
    idx(idx) {

  }

  var_index_t idx;
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

struct IRBinop : public IRExpr {
  IRBinop(var_index_t idx, var_index_t left, Binop op, var_index_t right) :
      IRExpr(idx), left_idx(left), op(op), right_idx(right) {

  }

  var_index_t left_idx;
  Binop op;
  var_index_t right_idx;

  std::string ToString() const final;
};

template<typename T>
struct IRImm : public IRExpr {
  static_assert(is_calyx_type_v<T>);

  IRImm(var_index_t idx, T value) :
      IRExpr(idx), value(value) {

  }

  T value;

  std::string ToString() const final;
};

struct IRUnop : public IRExpr {
  IRUnop(var_index_t idx, Unop op, var_index_t right) :
          IRExpr(idx), op(op), right_idx(right) {

  }

  Unop op;
  var_index_t right_idx;

  std::string ToString() const final;
};

template<typename T>
struct IRLoadCVar : public IRExpr {

  IRLoadCVar(var_index_t idx, var_index_t c_idx, i32 offset = 0) :
      IRExpr(idx), c_idx(c_idx), offset(offset) {

  }

  var_index_t c_idx;
  i32 offset;  // struct fields

  std::string ToString() const final { return ""; };
};

template<typename T>
struct IRStoreCVar : public IROp {

  IRStoreCVar(var_index_t c_idx, var_index_t src, i32 offset = 0) :
      IROp(Class::Store), c_idx(c_idx), src(src), offset(offset) {

  }

  var_index_t c_idx;
  var_index_t src;
  i32 offset;  // struct fields

  std::string ToString() const final { return ""; };
};

struct IRAllocateCVar : public IROp {
  IRAllocateCVar(var_index_t c_idx, u64 size) :
    IROp(Class::Stack), c_idx(c_idx), size(size) {

  }

  var_index_t c_idx;
  u64 size;

  std::string ToString() const final;
};

struct IRDeallocateCVar : public IROp {
  IRDeallocateCVar(var_index_t c_idx, u64 size) :
      IROp(Class::Stack), c_idx(c_idx), size(size) {

  }
  var_index_t c_idx;
  u64 size;

  std::string ToString() const final;
};

template<typename T>
struct IRLoadFromPointer : public IRExpr {
  static_assert(is_calyx_type_v<T>);

  IRLoadFromPointer(var_index_t idx, var_index_t ptr_idx, i32 offset = 0) :
          IRExpr(idx), ptr_idx(ptr_idx), offset(offset) {

  }

  var_index_t ptr_idx;
  i32 offset;

  std::string ToString() const final { return ""; }
};

template<typename T>
struct IRStoreToPointer : public IROp {
  static_assert(is_calyx_type_v<T>);

  IRStoreToPointer(var_index_t ptr_idx, var_index_t idx, i32 offset = 0) :
          IROp(Class::Store), idx(idx), ptr_idx(ptr_idx), offset(offset) {

  }

  var_index_t ptr_idx;
  var_index_t idx;
  i32 offset;

  std::string ToString() const final { return ""; }
};

struct IRReturn : public IROp {
  IRReturn(var_index_t idx) :
      IROp(Class::Branch), idx(idx) {

  }

  var_index_t idx;

  std::string ToString() const final;
};

}