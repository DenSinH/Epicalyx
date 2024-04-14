#pragma once

#include "Types.h"
#include "Variant.h"
#include "Packs.h"
#include "Containers.h"

#include <bit>
#include <vector>


namespace epi::calyx {

// IR var idx and Argument
using arg_list_t = std::vector<std::pair<var_index_t, Argument>>;

struct Directive {
  enum class Class {
    NoOp,
    Expression,  // includes loads
    Store,
    Stack,
    Branch,
    Select,
    Call,
    Return,
  };

  Directive(Class cls, size_t type_id) :
      cls(cls), type_id(type_id) {

  }

  virtual ~Directive() = default;

  Class cls;
  size_t type_id;

  // virtual std::string ToString() const = 0;
  // virtual void Emit(Backend& backend) = 0;
};

struct Expr : Directive {
  
  Expr(size_t type_id, var_index_t idx) :
      Directive(Class::Expression, type_id),
    idx(idx) {

  }

  var_index_t idx;
};

struct Branch : Directive {

  Branch(Class cls, size_t type_id) :
      Directive(cls, type_id) {

  }

  virtual std::vector<block_label_t> Destinations() const = 0;
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct Store : Directive {
  using src_t = calyx_upcast_t<T>;

  Store(size_t type_id, Operand<src_t> src) : 
      Directive(Class::Store, type_id),
      src{src} {

  }

  Operand<src_t> src;
};

struct NoOp : Directive {

  NoOp() : Directive(Class::NoOp, GetTID()) { }
  
  std::string ToString() const;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
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

enum class ShiftType {
  Left,
  Right,
};

enum class UnopType {
  Neg,
  BinNot
};

enum class CmpType {
  Eq, Ne, Lt, Le, Gt, Ge
};

template<typename To, typename From>
requires (
  is_calyx_arithmetic_ptr_type_v<From> && 
  (is_calyx_arithmetic_ptr_type_v<To> || is_calyx_small_type_v<To>)
)
struct Cast : Expr {
  using result_t = calyx_upcast_t<To>;
  using src_t = From;
  Cast(var_index_t idx, var_index_t right_idx) :
      Expr(GetTID(), idx), right_idx(right_idx) {

  }

  var_index_t right_idx;

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_arithmetic_type_v<T>)
struct Binop : Expr {
  using result_t = T;
  using src_t = T;
  Binop(var_index_t idx, var_index_t left, BinopType op, Operand<T> right) :
      Expr(GetTID(), idx), left_idx(left), op(op), right(right) {

  }

  var_index_t left_idx;
  BinopType op;
  Operand<T> right;

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_integral_type_v<T>)
struct Shift : Expr {
  using result_t = T;
  using src_t = T;
  // shift type changed when value is negative anyway
  using shift_t = u32;
  Shift(var_index_t idx, Operand<T> left, ShiftType op, Operand<shift_t> right) :
      Expr(GetTID(), idx), left(left), op(op), right(right) {

  }

  Operand<T> left;
  ShiftType op;
  Operand<shift_t> right;

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_arithmetic_ptr_type_v<T>)
struct Compare : Expr {
  using result_t = i32;
  using src_t = T;
  Compare(var_index_t idx, var_index_t left, CmpType op, Operand<T> right) :
          Expr(GetTID(), idx), left_idx(left), op(op), right(right) {

  }

  var_index_t left_idx;
  CmpType op;
  Operand<T> right;

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

struct UnconditionalBranch : Branch {
  UnconditionalBranch(block_label_t dest) :
      Branch(Class::Branch, GetTID()),
      dest{dest} {

  }

  block_label_t dest;
  std::vector<block_label_t> Destinations() const final {
    return {dest};
  }
  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_arithmetic_ptr_type_v<T>)
struct BranchCompare : Branch {
  using src_t = T;

  BranchCompare(block_label_t tdest, block_label_t fdest, var_index_t left, CmpType op, Operand<T> right) :
          Branch(Class::Branch, GetTID()), 
          tdest(tdest), fdest(fdest),
          left_idx(left), op(op), right(right) {

  }

  block_label_t tdest;
  block_label_t fdest;

  var_index_t left_idx;
  CmpType op;
  Operand<T> right;

  std::vector<block_label_t> Destinations() const final {
    return {tdest, fdest};
  }
  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_integral_type_v<T>)
struct AddToPointer : Expr {
  using result_t = Pointer;
  using offset_t = T;

  AddToPointer(var_index_t idx, Operand<Pointer> ptr, u64 stride, Operand<T> right) :
      Expr(GetTID(), idx), ptr(ptr), stride(stride), right(right) {

  }

  Operand<Pointer> ptr;
  u64 stride;
  Operand<offset_t> right;

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_type_v<T>)
struct Imm : Expr {
  using result_t = T;
  Imm(var_index_t idx, T value) :
      Expr(GetTID(), idx), value(value) {

  }

  T value;

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_arithmetic_type_v<T>)
struct Unop : Expr {
  using result_t = T;
  using src_t = T;
  Unop(var_index_t idx, UnopType op, var_index_t right) :
      Expr(GetTID(), idx), op(op), right_idx(right) {

  }

  UnopType op;
  var_index_t right_idx;

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct LoadLocal : Expr {
  using result_t = calyx_upcast_t<T>;
  LoadLocal(var_index_t idx, var_index_t loc_idx, i32 offset = 0) :
      Expr(GetTID(), idx), loc_idx(loc_idx), offset(offset) {

  }

  var_index_t loc_idx;
  i32 offset;  // struct fields

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

struct LoadLocalAddr : Expr {
  using result_t = Pointer;
  LoadLocalAddr(var_index_t idx, var_index_t loc_idx) :
          Expr(GetTID(), idx), loc_idx(loc_idx){

  }

  var_index_t loc_idx;

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct StoreLocal : public Store<T> {
  using src_t = Store<T>::src_t;

  StoreLocal(var_index_t loc_idx, Operand<src_t> src, i32 offset = 0) :
      Store<T>(GetTID(), src), 
      loc_idx(loc_idx), 
      offset(offset) {

  }

  var_index_t loc_idx;
  i32 offset;  // struct fields

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct LoadGlobal : Expr {
  using result_t = calyx_upcast_t<T>;
  LoadGlobal(var_index_t idx, std::string symbol, i32 offset = 0) :
      Expr(GetTID(), idx), symbol(std::move(symbol)), offset(offset) {

  }

  std::string symbol;
  i32 offset;  // struct fields

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

struct LoadGlobalAddr : Expr {
  using result_t = Pointer;
  LoadGlobalAddr(var_index_t idx, std::string symbol) :
          Expr(GetTID(), idx), symbol(std::move(symbol)){

  }

  std::string symbol;

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct StoreGlobal : public Store<T> {
  using src_t = Store<T>::src_t;

  StoreGlobal(std::string symbol, Operand<src_t> src, i32 offset = 0) :
      Store<T>(GetTID(), src),
      symbol(std::move(symbol)), 
      offset(offset) {

  }

  std::string symbol;
  i32 offset;  // struct fields

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};


template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct LoadFromPointer : Expr {
  using result_t = calyx_upcast_t<T>;
  
  LoadFromPointer(var_index_t idx, var_index_t ptr_idx, i32 offset = 0) :
      Expr(GetTID(), idx), ptr_idx(ptr_idx), offset(offset) {

  }

  var_index_t ptr_idx;
  i32 offset;

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct StoreToPointer : public Store<T> {
  using src_t = Store<T>::src_t;

  StoreToPointer(var_index_t ptr_idx, Operand<src_t> src, i32 offset = 0) :
      Store<T>(GetTID(), src), 
      ptr_idx(ptr_idx), 
      offset(offset) {

  }

  var_index_t ptr_idx;
  i32 offset;

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_return_types>)
struct Return : Directive {
  using src_t = T;
  using op_t = std::conditional_t<
    std::is_same_v<T, void>, 
    std::nullptr_t, 
    // hack needed for this to compile properly
    Operand<std::conditional_t<std::is_same_v<T, void>, i32, T>>
  >;

  Return() : Directive(Class::Return, GetTID()), val(0) { }
  
  Return(op_t val) :
    Directive(Class::Return, GetTID()), val(val) {

  }

  op_t val;

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_return_types>)
struct Call : Directive {
  using result_t = T;

  Call(var_index_t idx, var_index_t fn_idx, arg_list_t args, arg_list_t var_args) :
      Directive(Class::Call, GetTID()), idx(idx), fn_idx(fn_idx), args(std::move(args)), var_args(std::move(var_args)) {

  }

  var_index_t idx;
  var_index_t fn_idx;
  arg_list_t args;
  arg_list_t var_args;

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_return_types>)
struct CallLabel : Directive {
  using result_t = T;

  CallLabel(var_index_t idx, std::string label, arg_list_t args, arg_list_t var_args) :
    Directive(Class::Call, GetTID()), idx(idx), label(std::move(label)), args(std::move(args)), var_args(std::move(var_args)) {

  }

  var_index_t idx;
  std::string label;
  arg_list_t args;
  arg_list_t var_args;

  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

struct Select : Branch {
  using src_t = i64;

  Select(var_index_t idx) :
      Branch(Class::Select, GetTID()), idx(idx) {

  }

  // var is ALWAYS i64
  var_index_t idx;
  cotyl::unordered_map<i64, block_label_t> table{};
  block_label_t _default = 0;
  
  std::vector<block_label_t> Destinations() const final {
    std::vector<block_label_t> result{};
    result.reserve(table.size() + 1);
    for (const auto& [val, block_idx] : table) {
      result.emplace_back(block_idx);
    }
    if (_default) result.emplace_back(_default);
    return std::move(result);
  }
  std::string ToString() const;
  // void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

namespace detail {

template<typename To>
struct cast_to {
  template<typename From>
  using type = Cast<To, From>;
};

template<typename... Ts>
requires (std::is_base_of_v<Directive, Ts> && ...)
using any_directive_helper = cotyl::Variant<Directive, Ts...>;

template<typename... Ts>
requires (std::is_base_of_v<Expr, Ts> && ...)
using any_expr_helper = cotyl::Variant<Expr, Ts...>;

using expr_pack = cotyl::flatten_pack<
  cotyl::map_types_t<Binop, calyx_arithmetic_types>,
  cotyl::map_types_t<Shift, calyx_integral_types>,
  cotyl::map_types_t<Compare, calyx_arithmetic_ptr_types>,
  cotyl::map_types_t<AddToPointer, calyx_integral_types>,
  cotyl::map_types_t<Unop, calyx_arithmetic_types>,
  cotyl::map_types_t<Imm, calyx_arithmetic_ptr_types>,
  cotyl::map_types_t<LoadLocal, calyx_memory_types>,
  LoadLocalAddr,
  cotyl::map_types_t<LoadGlobal, calyx_memory_types>,
  LoadGlobalAddr,
  cotyl::map_types_t<LoadFromPointer, calyx_memory_types>,
  cotyl::map_types_t<detail::cast_to<i8>::type, calyx_arithmetic_ptr_types>,
  cotyl::map_types_t<detail::cast_to<u8>::type, calyx_arithmetic_ptr_types>,
  cotyl::map_types_t<detail::cast_to<i16>::type, calyx_arithmetic_ptr_types>,
  cotyl::map_types_t<detail::cast_to<u16>::type, calyx_arithmetic_ptr_types>,
  cotyl::map_types_t<detail::cast_to<i32>::type, calyx_arithmetic_ptr_types>,
  cotyl::map_types_t<detail::cast_to<u32>::type, calyx_arithmetic_ptr_types>,
  cotyl::map_types_t<detail::cast_to<i64>::type, calyx_arithmetic_ptr_types>,
  cotyl::map_types_t<detail::cast_to<u64>::type, calyx_arithmetic_ptr_types>,
  cotyl::map_types_t<detail::cast_to<float>::type, calyx_arithmetic_ptr_types>,
  cotyl::map_types_t<detail::cast_to<double>::type, calyx_arithmetic_ptr_types>,
  cotyl::map_types_t<detail::cast_to<Pointer>::type, calyx_arithmetic_ptr_types>
>;

using directive_pack = cotyl::flatten_pack<
  NoOp,
  expr_pack,
  UnconditionalBranch,
  cotyl::map_types_t<BranchCompare, calyx_arithmetic_ptr_types>,
  Select,
  cotyl::map_types_t<StoreLocal, calyx_memory_types>,
  cotyl::map_types_t<StoreGlobal, calyx_memory_types>,
  cotyl::map_types_t<StoreToPointer, calyx_memory_types>,
  cotyl::map_types_t<Call, calyx_return_types>,
  cotyl::map_types_t<CallLabel, calyx_return_types>,
  cotyl::map_types_t<Return, calyx_return_types>
>;

}

using AnyExpr = cotyl::map_pack_t<
  detail::any_expr_helper,
  detail::expr_pack
>;

using AnyDirective = cotyl::map_pack_t<
  detail::any_directive_helper,
  detail::directive_pack
>;

}