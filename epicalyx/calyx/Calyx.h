#pragma once

#include "Default.h"
#include "Is.h"

#include <memory>
#include <string>


namespace epi::calyx {

struct Backend;

using var_index_t = u64;
using block_label_t = u64;

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
template<> struct calyx_upcast<u8> { using type = i32; };
template<> struct calyx_upcast<i16> { using type = i32; };
template<> struct calyx_upcast<u16> { using type = i32; };
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


struct Directive {
  enum class Class {
    Expression,  // includes loads
    Store,
    Stack,
    Branch,
  };

  Directive(Class cls) :
      cls(cls) {

  }

  virtual ~Directive() = default;

  Class cls;

  virtual std::string ToString() const = 0;
  virtual void Emit(Backend& backend) = 0;
};

template<typename T>
struct Expr : Directive {
  static_assert(is_calyx_type_v<T>);
  
  Expr(var_index_t idx) :
      Directive(Class::Expression),
    idx(idx) {

  }

  var_index_t idx;
};

struct Branch : Directive {

  Branch(block_label_t dest) :
      Directive(Class::Branch),
      dest(dest) {

  }

  block_label_t dest;
};

using pDirective = std::unique_ptr<Directive>;

enum class BinopType {
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  BinAnd,
  BinOr,
  BinXor,
};

enum class ShiftType {
  Left,
  Right,
};

enum class UnopType {
  Neg,
  BinNot
};

enum class CmpType {
  // signed or unsigned is from the type of the cmp instruction
  Eq, Ne, Lt, Le, Gt, Ge
};

template<typename To, typename From>
struct Cast : Expr<calyx_upcast_t<To>> {
  Cast(var_index_t idx, var_index_t right_idx) :
      Expr<calyx_upcast_t<To>>(idx), right_idx(right_idx) {

  }

  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
struct Binop : Expr<T> {
  static_assert(is_calyx_type_v<T>);

  Binop(var_index_t idx, var_index_t left, BinopType op, var_index_t right) :
      Expr<T>(idx), left_idx(left), op(op), right_idx(right) {

  }

  var_index_t left_idx;
  BinopType op;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
struct BinopImm : Expr<T> {
  static_assert(is_calyx_type_v<T>);

  BinopImm(var_index_t idx, var_index_t left, BinopType op, T right) :
      Expr<T>(idx), left_idx(left), op(op), right(right) {

  }

  var_index_t left_idx;
  BinopType op;
  T right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
struct Shift : Expr<T> {
  static_assert(is_calyx_type_v<T>);

  Shift(var_index_t idx, var_index_t left, ShiftType op, var_index_t right) :
      Expr<T>(idx), left_idx(left), op(op), right_idx(right) {

  }

  var_index_t left_idx;
  ShiftType op;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

struct UnconditionalBranch : Branch {
  UnconditionalBranch(block_label_t dest) :
      Branch(dest) {

  }

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
struct BranchCompare : Branch {
  static_assert(is_calyx_type_v<T>);

  BranchCompare(block_label_t dest, var_index_t left, CmpType op, var_index_t right) :
      Branch(dest), left_idx(left), op(op), right_idx(right) {

  }

  var_index_t left_idx;
  CmpType op;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
struct BranchCompareImm : Branch {
  static_assert(is_calyx_type_v<T>);

  BranchCompareImm(block_label_t dest, var_index_t left, CmpType op, T right) :
    Branch(dest), left_idx(left), op(op), right(right) {

  }


  var_index_t left_idx;
  CmpType op;
  T right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
struct AddToPointer : Expr<Pointer> {
  static_assert(is_calyx_type_v<Pointer>);

  AddToPointer(var_index_t idx, var_index_t left, u64 stride, var_index_t right) :
      Expr<Pointer>(idx), ptr_idx(left), stride(stride), right_idx(right) {

  }

  var_index_t ptr_idx;
  u64 stride;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
struct Imm : Expr<T> {
  static_assert(is_calyx_type_v<T>);

  Imm(var_index_t idx, T value) :
      Expr<T>(idx), value(value) {

  }

  T value;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
struct Unop : Expr<T> {
  static_assert(is_calyx_type_v<T>);

  Unop(var_index_t idx, UnopType op, var_index_t right) :
      Expr<T>(idx), op(op), right_idx(right) {

  }

  UnopType op;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
struct LoadCVar : Expr<calyx_upcast_t<T>> {

  LoadCVar(var_index_t idx, var_index_t c_idx, i32 offset = 0) :
      Expr<calyx_upcast_t<T>>(idx), c_idx(c_idx), offset(offset) {

  }

  var_index_t c_idx;
  i32 offset;  // struct fields

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

struct LoadCVarAddr : Expr<Pointer> {

  LoadCVarAddr(var_index_t idx, var_index_t c_idx) :
          Expr<Pointer>(idx), c_idx(c_idx){

  }

  var_index_t c_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
struct StoreCVar : Expr<calyx_upcast_t<T>> {

  StoreCVar(var_index_t idx, var_index_t c_idx, var_index_t src, i32 offset = 0) :
      Expr<calyx_upcast_t<T>>(idx), c_idx(c_idx), src(src), offset(offset) {

  }

  var_index_t c_idx;
  var_index_t src;
  i32 offset;  // struct fields

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

struct AllocateCVar : Directive {
  AllocateCVar(var_index_t c_idx, u64 size) :
      Directive(Class::Stack), c_idx(c_idx), size(size) {

  }

  var_index_t c_idx;
  u64 size;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

struct DeallocateCVar : Directive {
  DeallocateCVar(var_index_t c_idx, u64 size) :
      Directive(Class::Stack), c_idx(c_idx), size(size) {

  }
  var_index_t c_idx;
  u64 size;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
struct LoadFromPointer : Expr<T> {
  static_assert(is_calyx_type_v<T>);

  LoadFromPointer(var_index_t idx, var_index_t ptr_idx, i32 offset = 0) :
      Expr<T>(idx), ptr_idx(ptr_idx), offset(offset) {

  }

  var_index_t ptr_idx;
  i32 offset;

  std::string ToString() const final { return ""; }
  void Emit(Backend& backend) final;
};

template<typename T>
struct StoreToPointer : Directive {
  static_assert(is_calyx_type_v<T>);

  StoreToPointer(var_index_t ptr_idx, var_index_t idx, i32 offset = 0) :
          Directive(Class::Store), idx(idx), ptr_idx(ptr_idx), offset(offset) {

  }

  var_index_t ptr_idx;
  var_index_t idx;
  i32 offset;

  std::string ToString() const final { return ""; }
  void Emit(Backend& backend) final;
};

template<typename T>
struct Return : Directive {
  static_assert(is_calyx_type_v<T>);

  Return(var_index_t idx) :
          Directive(Class::Branch), idx(idx) {

  }

  var_index_t idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

}