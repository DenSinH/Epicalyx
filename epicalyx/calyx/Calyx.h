#pragma once

#include "Default.h"
#include "Is.h"

#include <memory>
#include <string>


namespace epi::calyx {

using var_index_t = u64;

struct Pointer;
struct Struct;

template<typename T>
struct is_calyx_type {
  static constexpr bool value = epi::cotyl::is_in_v<T, i32, u32, i64, u64, float, double, Struct, Pointer>;
};
template<typename T>
constexpr bool is_calyx_type_v = is_calyx_type<T>::value;

template<typename T> struct calyx_upcast { using type = T; };
template<> struct calyx_upcast<i8> { using type = i32; };
template<> struct calyx_upcast<u8> { using type = u32; };
template<> struct calyx_upcast<i16> { using type = i32; };
template<> struct calyx_upcast<u16> { using type = u32; };
template<typename T>
using calyx_upcast_t = typename calyx_upcast<T>::type;

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

struct Var {
  enum class Type {
    I32, U32, I64, U64, Float, Double, Pointer, Struct
  };

  Var(Type type, u64 stride = 0) :
      type(type), stride(stride) {

  }

  Type type;
  u64 stride;  // for pointers
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

template<typename T>
struct IRExpr : IROp {
  static_assert(is_calyx_type_v<T>);
  
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

template<typename To, typename From>
struct IRCast : IRExpr<calyx_upcast_t<To>> {
  IRCast(var_index_t idx, var_index_t right_idx) :
      IRExpr<calyx_upcast_t<To>>(idx), right_idx(right_idx) {

  }

  var_index_t right_idx;

  std::string ToString() const final;
};

template<typename T>
struct IRBinop : IRExpr<T> {
  static_assert(is_calyx_type_v<T>);

  IRBinop(var_index_t idx, var_index_t left, Binop op, var_index_t right) :
      IRExpr<T>(idx), left_idx(left), op(op), right_idx(right) {

  }

  var_index_t left_idx;
  Binop op;
  var_index_t right_idx;

  std::string ToString() const final;
};

template<typename T>
struct IRAddToPointer : IRExpr<Pointer> {
  static_assert(is_calyx_type_v<Pointer>);

  IRAddToPointer(var_index_t idx, var_index_t left, u64 stride, var_index_t right) :
      IRExpr<Pointer>(idx), ptr_idx(left), stride(stride), right_idx(right) {

  }

  var_index_t ptr_idx;
  u64 stride;
  var_index_t right_idx;

  std::string ToString() const final;
};

template<typename T>
struct IRImm : IRExpr<T> {
  static_assert(is_calyx_type_v<T>);

  IRImm(var_index_t idx, T value) :
      IRExpr<T>(idx), value(value) {

  }

  T value;

  std::string ToString() const final;
};

template<typename T>
struct IRUnop : IRExpr<T> {
  static_assert(is_calyx_type_v<T>);

  IRUnop(var_index_t idx, Unop op, var_index_t right) :
      IRExpr<T>(idx), op(op), right_idx(right) {

  }

  Unop op;
  var_index_t right_idx;

  std::string ToString() const final;
};

template<typename T>
struct IRLoadCVar : IRExpr<calyx_upcast_t<T>> {

  IRLoadCVar(var_index_t idx, var_index_t c_idx, i32 offset = 0) :
      IRExpr<calyx_upcast_t<T>>(idx), c_idx(c_idx), offset(offset) {

  }

  var_index_t c_idx;
  i32 offset;  // struct fields

  std::string ToString() const final;
};

struct IRLoadCVarAddr : IRExpr<Pointer> {

  IRLoadCVarAddr(var_index_t idx, var_index_t c_idx) :
          IRExpr<Pointer>(idx), c_idx(c_idx){

  }

  var_index_t c_idx;

  std::string ToString() const final;
};

template<typename T>
struct IRStoreCVar : IRExpr<calyx_upcast_t<T>> {

  IRStoreCVar(var_index_t idx, var_index_t c_idx, var_index_t src, i32 offset = 0) :
      IRExpr<calyx_upcast_t<T>>(idx), c_idx(c_idx), src(src), offset(offset) {

  }

  var_index_t c_idx;
  var_index_t src;
  i32 offset;  // struct fields

  std::string ToString() const final;
};

struct IRAllocateCVar : IROp {
  IRAllocateCVar(var_index_t c_idx, u64 size) :
    IROp(Class::Stack), c_idx(c_idx), size(size) {

  }

  var_index_t c_idx;
  u64 size;

  std::string ToString() const final;
};

struct IRDeallocateCVar : IROp {
  IRDeallocateCVar(var_index_t c_idx, u64 size) :
      IROp(Class::Stack), c_idx(c_idx), size(size) {

  }
  var_index_t c_idx;
  u64 size;

  std::string ToString() const final;
};

template<typename T>
struct IRLoadFromPointer : IRExpr<T> {
  static_assert(is_calyx_type_v<T>);

  IRLoadFromPointer(var_index_t idx, var_index_t ptr_idx, i32 offset = 0) :
      IRExpr<T>(idx), ptr_idx(ptr_idx), offset(offset) {

  }

  var_index_t ptr_idx;
  i32 offset;

  std::string ToString() const final { return ""; }
};

template<typename T>
struct IRStoreToPointer : IROp {
  static_assert(is_calyx_type_v<T>);

  IRStoreToPointer(var_index_t ptr_idx, var_index_t idx, i32 offset = 0) :
          IROp(Class::Store), idx(idx), ptr_idx(ptr_idx), offset(offset) {

  }

  var_index_t ptr_idx;
  var_index_t idx;
  i32 offset;

  std::string ToString() const final { return ""; }
};

struct IRReturn : IROp {
  IRReturn(var_index_t idx) :
      IROp(Class::Branch), idx(idx) {

  }

  var_index_t idx;

  std::string ToString() const final;
};

}