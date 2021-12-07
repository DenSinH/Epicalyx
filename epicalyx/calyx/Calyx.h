#pragma once

#include "Default.h"
#include "Is.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>


namespace epi::calyx {

struct Backend;

using var_index_t = u64;
using block_label_t = u64;

struct Pointer {
  Pointer() : value(0) { }
  Pointer(u64 value) : value(value) { }

  u64 value;
};
struct Struct;

template<typename T>
constexpr bool is_calyx_type_v = epi::cotyl::is_in_v<T, i32, u32, i64, u64, float, double, Struct, Pointer>;
template<typename T>
constexpr bool is_calyx_integral_type_v = epi::cotyl::is_in_v<T, i32, u32, i64, u64>;

template<typename T> struct calyx_upcast { using type = T; };
template<> struct calyx_upcast<i8> { using type = i32; };
template<> struct calyx_upcast<u8> { using type = i32; };  // todo: fix this in CType (cast to i32, should be u32)
template<> struct calyx_upcast<i16> { using type = i32; };
template<> struct calyx_upcast<u16> { using type = i32; };
template<typename T>
using calyx_upcast_t = typename calyx_upcast<T>::type;

template<typename T>
struct calyx_imm_type {
  using type = T;
};

template<>
struct calyx_imm_type<Pointer> {
  using type = u64;
};

template<typename T>
using calyx_imm_type_t = typename calyx_imm_type<T>::type;

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
    UnconditionalBranch,
    Return,
    Select,
  };

  Directive(Class cls) :
      cls(cls) {

  }

  virtual ~Directive() = default;

  Class cls;

  virtual std::string ToString() const = 0;
  virtual void Emit(Backend& backend) = 0;
};

using pDirective = std::unique_ptr<Directive>;
using Program = std::vector<std::vector<calyx::pDirective>>;

template<typename T>
requires (is_calyx_type_v<T>)
struct Expr : Directive {
  
  Expr(var_index_t idx) :
      Directive(Class::Expression),
    idx(idx) {

  }

  var_index_t idx;
};

struct Branch : Directive {

  Branch(Class cls, block_label_t dest) :
      Directive(cls),
      dest(dest) {

  }

  block_label_t dest;
};

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

enum class PtrAddType {
  Add,
  Sub,
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
requires (is_calyx_type_v<T>)
struct Binop : Expr<T> {

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
requires (is_calyx_type_v<T>)
struct BinopImm : Expr<T> {

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
requires (is_calyx_type_v<T>)
struct Shift : Expr<T> {

  Shift(var_index_t idx, var_index_t left, ShiftType op, var_index_t right) :
      Expr<T>(idx), left_idx(left), op(op), right_idx(right) {

  }

  var_index_t left_idx;
  ShiftType op;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
requires (is_calyx_type_v<T>)
struct ShiftImm : Expr<T> {

  ShiftImm(var_index_t idx, var_index_t left, ShiftType op, T right) :
      Expr<T>(idx), left_idx(left), op(op), right(right) {

  }

  var_index_t left_idx;
  ShiftType op;
  T right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
requires (is_calyx_type_v<T>)
struct Compare : Expr<i32> {

  Compare(var_index_t idx, var_index_t left, CmpType op, var_index_t right) :
          Expr<i32>(idx), left_idx(left), op(op), right_idx(right) {

  }

  var_index_t left_idx;
  CmpType op;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
requires (is_calyx_type_v<T>)
struct CompareImm : Expr<i32> {

  CompareImm(var_index_t idx, var_index_t left, CmpType op, T right) :
          Expr<i32>(idx), left_idx(left), op(op), right(right) {

  }

  var_index_t left_idx;
  CmpType op;
  T right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

struct UnconditionalBranch : Branch {
  UnconditionalBranch(block_label_t dest) :
      Branch(Class::UnconditionalBranch, dest) {

  }

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
requires (is_calyx_type_v<T>)
struct BranchCompare : Branch {

  BranchCompare(block_label_t dest, var_index_t left, CmpType op, var_index_t right) :
      Branch(Class::Branch, dest), left_idx(left), op(op), right_idx(right) {

  }

  var_index_t left_idx;
  CmpType op;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
requires (is_calyx_type_v<T>)
struct BranchCompareImm : Branch {

  BranchCompareImm(block_label_t dest, var_index_t left, CmpType op, calyx_imm_type_t<T> right) :
    Branch(Class::Branch, dest), left_idx(left), op(op), right(right) {

  }

  var_index_t left_idx;
  CmpType op;
  calyx_imm_type_t<T> right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
requires (is_calyx_integral_type_v<T>)
struct AddToPointer : Expr<Pointer> {

  AddToPointer(var_index_t idx, var_index_t ptr, PtrAddType op, u64 stride, var_index_t right) :
      Expr<Pointer>(idx), ptr_idx(ptr), op(op), stride(stride), right_idx(right) {

  }

  var_index_t ptr_idx;
  PtrAddType op;
  u64 stride;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

struct AddToPointerImm : Expr<Pointer> {

  AddToPointerImm(var_index_t idx, var_index_t ptr, u64 stride, i64 right) :
      Expr<Pointer>(idx), ptr_idx(ptr), stride(stride), right(right) {

  }

  var_index_t ptr_idx;
  u64 stride;
  i64 right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
requires (is_calyx_type_v<T>)
struct Imm : Expr<T> {

  Imm(var_index_t idx, T value) :
      Expr<T>(idx), value(value) {

  }

  T value;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
requires (is_calyx_type_v<T>)
struct Unop : Expr<T> {

  Unop(var_index_t idx, UnopType op, var_index_t right) :
      Expr<T>(idx), op(op), right_idx(right) {

  }

  UnopType op;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
struct LoadLocal : Expr<calyx_upcast_t<T>> {

  LoadLocal(var_index_t idx, var_index_t c_idx, i32 offset = 0) :
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
struct StoreLocal : Expr<calyx_upcast_t<T>> {

  StoreLocal(var_index_t idx, var_index_t c_idx, var_index_t src, i32 offset = 0) :
      Expr<calyx_upcast_t<T>>(idx), c_idx(c_idx), src(src), offset(offset) {

  }

  var_index_t c_idx;
  var_index_t src;
  i32 offset;  // struct fields

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

struct AllocateLocal : Directive {
  AllocateLocal(var_index_t c_idx, u64 size) :
      Directive(Class::Stack), c_idx(c_idx), size(size) {

  }

  var_index_t c_idx;
  u64 size;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

struct DeallocateLocal : Directive {
  DeallocateLocal(var_index_t c_idx, u64 size) :
      Directive(Class::Stack), c_idx(c_idx), size(size) {

  }
  var_index_t c_idx;
  u64 size;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

template<typename T>
requires (is_calyx_type_v<T>)
struct LoadFromPointer : Expr<T> {

  LoadFromPointer(var_index_t idx, var_index_t ptr_idx, i32 offset = 0) :
      Expr<T>(idx), ptr_idx(ptr_idx), offset(offset) {

  }

  var_index_t ptr_idx;
  i32 offset;

  std::string ToString() const final { return ""; }
  void Emit(Backend& backend) final;
};

template<typename T>
requires (is_calyx_type_v<T>)
struct StoreToPointer : Directive {

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
requires (is_calyx_type_v<T>)
struct Return : Directive {

  Return(var_index_t idx) :
        Directive(Class::Return), idx(idx) {

  }

  var_index_t idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

struct Select : Directive {

  Select(var_index_t idx) :
        Directive(Class::Select), idx(idx) {

  }

  var_index_t idx;
  std::unordered_map<i64, block_label_t> table{};
  block_label_t _default = 0;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
};

}