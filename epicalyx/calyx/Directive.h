#pragma once

#include "Types.h"
#include "Variant.h"
#include "Packs.h"
#include "Containers.h"
#include "Stringify.h"

#include <bit>
#include <vector>


namespace epi::calyx {

struct Directive {

  Directive() = default;

};

struct Expr : Directive {
  
  Expr(var_index_t idx) :
      Directive{}, idx(idx) {

  }

  var_index_t idx;
};

struct Branch : Directive {

  Branch() = default;

  virtual std::vector<block_label_t> Destinations() const = 0;
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct Store : Directive {
  using src_t = calyx_upcast_t<T>;

  Store(Operand<src_t> src) : 
      Directive{}, src{src} {

  }

  Operand<src_t> src;
};

struct NoOp : Directive {

  NoOp() = default;
  
  std::string ToString() const;
};

enum class BinopType : u32 {
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  BinAnd,
  BinOr,
  BinXor,
};

enum class ShiftType : u32 {
  Left,
  Right,
};

enum class UnopType : u32 {
  Neg,
  BinNot
};

enum class CmpType : u32 {
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
      Expr{idx}, right_idx{right_idx} {

  }

  var_index_t right_idx;

  std::string ToString() const;
};

template<typename T>
requires (is_calyx_arithmetic_type_v<T>)
struct Binop : Expr {
  using result_t = T;
  using src_t = T;
  Binop(var_index_t idx, var_index_t left, BinopType op, Operand<T> right) :
      Expr{idx}, left_idx{left}, op{op}, right{right} {

  }

  var_index_t left_idx;
  BinopType op;
  Operand<T> right;

  std::string ToString() const;
};

template<typename T>
requires (is_calyx_integral_type_v<T>)
struct Shift : Expr {
  using result_t = T;
  using src_t = T;
  // shift type changed when value is negative anyway
  using shift_t = u32;
  Shift(var_index_t idx, Operand<T> left, ShiftType op, Operand<shift_t> right) :
      Expr{idx}, left{left}, op{op}, right{right} {

  }

  Operand<T> left;
  ShiftType op;
  Operand<shift_t> right;

  std::string ToString() const;
};

template<typename T>
requires (is_calyx_arithmetic_ptr_type_v<T>)
struct Compare : Expr {
  using result_t = i32;
  using src_t = T;
  Compare(var_index_t idx, var_index_t left, CmpType op, Operand<T> right) :
          Expr{idx}, left_idx{left}, op{op}, right{right} {

  }

  var_index_t left_idx;
  CmpType op;
  Operand<T> right;

  std::string ToString() const;
};

struct UnconditionalBranch : Branch {
  UnconditionalBranch(block_label_t dest) :
      Branch{}, dest{dest} {

  }

  block_label_t dest;
  std::vector<block_label_t> Destinations() const final {
    return {dest};
  }
  std::string ToString() const;
};

template<typename T>
requires (is_calyx_arithmetic_ptr_type_v<T>)
struct BranchCompare : Branch {
  using src_t = T;

  BranchCompare(block_label_t tdest, block_label_t fdest, var_index_t left, CmpType op, Operand<T> right) :
          Branch{}, 
          tdest{tdest}, fdest{fdest},
          left_idx{left}, op{op}, right{right} {

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
};

template<typename T>
requires (is_calyx_integral_type_v<T>)
struct AddToPointer : Expr {
  using result_t = Pointer;
  using offset_t = T;

  AddToPointer(var_index_t idx, Operand<Pointer> ptr, u32 stride, Operand<T> right) :
      Expr{idx}, stride{stride}, ptr{ptr}, right{right} {

  }

  u32 stride;
  Operand<Pointer> ptr;
  Operand<offset_t> right;

  std::string ToString() const;
};

template<typename T>
requires (is_calyx_type_v<T>)
struct Imm : Expr {
  using result_t = T;
  Imm(var_index_t idx, T value) :
      Expr{idx}, value{value} {

  }

  T value;

  std::string ToString() const;
};

template<typename T>
requires (is_calyx_arithmetic_type_v<T>)
struct Unop : Expr {
  using result_t = T;
  using src_t = T;
  Unop(var_index_t idx, UnopType op, var_index_t right) :
      Expr{idx}, op{op}, right_idx{right} {

  }

  UnopType op;
  var_index_t right_idx;

  std::string ToString() const;
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct LoadLocal : Expr {
  using result_t = calyx_upcast_t<T>;
  LoadLocal(var_index_t idx, var_index_t loc_idx, i32 offset = 0) :
      Expr{idx}, loc_idx{loc_idx}, offset{offset} {

  }

  var_index_t loc_idx;
  i32 offset;  // struct fields

  std::string ToString() const;
};

struct LoadLocalAddr : Expr {
  using result_t = Pointer;
  LoadLocalAddr(var_index_t idx, var_index_t loc_idx) :
          Expr{idx}, loc_idx{loc_idx} {

  }

  var_index_t loc_idx;

  std::string ToString() const;
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct StoreLocal : public Store<T> {
  using src_t = Store<T>::src_t;

  StoreLocal(var_index_t loc_idx, Operand<src_t> src, i32 offset = 0) :
      Store<T>{src}, 
      loc_idx{loc_idx}, 
      offset{offset} {

  }

  var_index_t loc_idx;
  i32 offset;  // struct fields

  std::string ToString() const;
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct LoadGlobal : Expr {
  using result_t = calyx_upcast_t<T>;
  LoadGlobal(var_index_t idx, cotyl::CString&& symbol, i32 offset = 0) :
      Expr{idx}, offset{offset}, symbol{std::move(symbol)} {

  }

  i32 offset;  // struct fields
  cotyl::CString symbol;

  std::string ToString() const;
};

struct LoadGlobalAddr : Expr {
  using result_t = Pointer;
  LoadGlobalAddr(var_index_t idx, cotyl::CString&& symbol) :
          Expr{idx}, symbol{std::move(symbol)} {

  }

  cotyl::CString symbol;

  std::string ToString() const;
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct StoreGlobal : public Store<T> {
  using src_t = Store<T>::src_t;

  StoreGlobal(cotyl::CString&& symbol, Operand<src_t> src, i32 offset = 0) :
      Store<T>{src},
      symbol{std::move(symbol)},
      offset{offset} {

  }

  cotyl::CString symbol;
  i32 offset;  // struct fields

  std::string ToString() const;
};


template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct LoadFromPointer : Expr {
  using result_t = calyx_upcast_t<T>;
  
  LoadFromPointer(var_index_t idx, var_index_t ptr_idx, i32 offset = 0) :
      Expr{idx}, ptr_idx{ptr_idx}, offset{offset} {

  }

  var_index_t ptr_idx;
  i32 offset;

  std::string ToString() const;
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
struct StoreToPointer : public Store<T> {
  using src_t = Store<T>::src_t;

  StoreToPointer(var_index_t ptr_idx, Operand<src_t> src, i32 offset = 0) :
      Store<T>{src}, 
      ptr_idx{ptr_idx}, 
      offset{offset} {

  }

  var_index_t ptr_idx;
  i32 offset;

  std::string ToString() const;
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

  Return() : Directive{}, val{0} { }
  
  Return(op_t val) : Directive{}, val{val} {

  }

  op_t val;

  std::string ToString() const;
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_return_types>)
struct Call : Directive {
  using result_t = T;

  Call(var_index_t idx, var_index_t fn_idx, ArgData&& args) :
      Directive{}, idx{idx}, fn_idx{fn_idx}, args{std::make_shared<ArgData>(std::move(args))} {

  }

  var_index_t idx;
  var_index_t fn_idx;
  std::shared_ptr<ArgData> args;

  std::string ToString() const;
};

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_return_types>)
struct CallLabel : Directive {
  using result_t = T;

  CallLabel(var_index_t idx, cotyl::CString&& label, ArgData&& args) :
    Directive{}, idx{idx}, label{std::move(label)}, args{std::make_shared<ArgData>(std::move(args))} {

  }

  var_index_t idx;
  cotyl::CString label;
  std::shared_ptr<ArgData> args;

  std::string ToString() const;
};

struct Select : Branch {
  using src_t = i64;
  using table_t = cotyl::unordered_map<i64, block_label_t>;

  Select(var_index_t idx) : Branch{}, idx{idx} { }

  // var is ALWAYS an i64
  var_index_t idx;
  block_label_t _default = 0;
  std::shared_ptr<table_t> table{std::make_shared<table_t>()};
  
  std::vector<block_label_t> Destinations() const final {
    std::vector<block_label_t> result{};
    result.reserve(table->size() + 1);
    for (const auto& [val, block_idx] : *table) {
      result.emplace_back(block_idx);
    }
    if (_default) result.emplace_back(_default);
    return std::move(result);
  }
  std::string ToString() const;
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

using any_expr_t = cotyl::map_pack_t<
  detail::any_expr_helper,
  detail::expr_pack
>;

using any_directive_t = cotyl::map_pack_t<
  detail::any_directive_helper,
  detail::directive_pack
>;

}

struct AnyExpr : detail::any_expr_t {
  using detail::any_expr_t::any_expr_t;
};

STRINGIFY_METHOD(AnyExpr);

struct AnyDirective : detail::any_directive_t {
  using detail::any_directive_t::any_directive_t;
};

STRINGIFY_METHOD(AnyDirective);

bool IsBlockEnd(const AnyDirective& dir);

template<typename T>
static bool IsType(const calyx::AnyDirective& directive) {
  return directive.holds_alternative<T>();
}

static bool IsSameType(const calyx::AnyDirective& dir1, const calyx::AnyDirective& dir2) {
  return dir1.index() == dir2.index();
}

}