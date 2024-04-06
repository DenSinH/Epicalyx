#pragma once

#include "Default.h"
#include "Is.h"
#include "Containers.h"

#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <bit>


namespace epi::calyx {

struct Backend;

using var_index_t = u64;
using block_label_t = u64;
using program_pos_t = std::pair<block_label_t, int>;

struct label_offset_t {
  std::string label;
  i64 offset;
};

struct Pointer {
  Pointer() : value(0) { }
  Pointer(i64 value) : value(value) { }

  i64 value;
};

static_assert(sizeof(Pointer) == sizeof(i64));

struct Struct;

template<typename T>
constexpr bool is_calyx_small_type_v = epi::cotyl::is_in_v<T, i8, u8, i16, u16>;
template<typename T>
constexpr bool is_calyx_integral_type_v = epi::cotyl::is_in_v<T, i32, u32, i64, u64>;
template<typename T>
constexpr bool is_calyx_arithmetic_type_v = is_calyx_integral_type_v<T> || epi::cotyl::is_in_v<T, float, double>;
template<typename T>
constexpr bool is_calyx_arithmetic_ptr_type_v = is_calyx_arithmetic_type_v<T> || std::is_same_v<T, Pointer>;
template<typename T>
constexpr bool is_calyx_type_v = is_calyx_arithmetic_ptr_type_v<T> || std::is_same_v<T, Struct>;

template<typename T> struct calyx_upcast { using type = T; };
template<> struct calyx_upcast<i8> { using type = i32; };
template<> struct calyx_upcast<u8> { using type = i32; };  // todo: fix this in CType (cast to i32, should be u32)
template<> struct calyx_upcast<i16> { using type = i32; };
template<> struct calyx_upcast<u16> { using type = i32; };
template<typename T>
using calyx_upcast_t = typename calyx_upcast<T>::type;

#define calyx_op_type(op) typename std::decay_t<decltype(op)>

struct Argument {
  enum class Type {
    I8, U8, I16, U16, I32, U32, I64, U64, Float, Double, Pointer, Struct
  };

  Argument() = default;
  
  Argument(Type type, var_index_t arg_idx, bool variadic = false) :
          type(type), arg_idx(arg_idx), variadic(variadic), stride(0) {

  }

  Argument(Type type, var_index_t arg_idx, u64 stride, bool variadic = false) :
          type(type), arg_idx(arg_idx), variadic(variadic), stride(stride) {

  }

  Type type;
  var_index_t arg_idx;
  bool variadic;
  union {
    u64 stride;  // for pointers
    u64 size;    // for structs
  };
};

// IR var idx and Argument
using arg_list_t = std::vector<std::pair<var_index_t, Argument>>;

struct Var {
  enum class Type {
    I32, U32, I64, U64, Float, Double, Pointer, Struct
  };

  Var(Type type, u64 stride = 0) :
      type(type), stride(stride) {

  }

  Type type;
  union {
    u64 stride;  // for pointers
    u64 size;    // for structs
  };
};


struct Directive {
  enum class Class {
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

  virtual std::string ToString() const = 0;
  virtual void Emit(Backend& backend) = 0;
};

using pDirective = std::unique_ptr<Directive>;

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

struct Program {
  using block_t = std::vector<pDirective>;
  using global_t = std::variant<i8, u8, i16, u16, i32, u32, i64, u64, float, double, Pointer, label_offset_t>;

  // program code
  // block 0 is special
  cotyl::unordered_map<block_label_t, block_t> blocks{};

  // function symbols -> entrypoint block ID
  cotyl::unordered_map<std::string, calyx::block_label_t> functions{};

  // string constants
  std::vector<std::string> strings{};

  // global variable sizes
  cotyl::unordered_map<std::string, global_t> globals{};

  size_t Hash() const;
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
  // signed or unsigned is from the type of the cmp instruction
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

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_arithmetic_type_v<T>)
struct Binop : Expr {
  using result_t = T;
  using src_t = T;
  Binop(var_index_t idx, var_index_t left, BinopType op, var_index_t right) :
      Expr(GetTID(), idx), left_idx(left), op(op), right_idx(right) {

  }

  var_index_t left_idx;
  BinopType op;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_arithmetic_type_v<T>)
struct BinopImm : Expr {
  using result_t = T;
  using src_t = T;
  BinopImm(var_index_t idx, var_index_t left, BinopType op, T right) :
      Expr(GetTID(), idx), left_idx(left), op(op), right(right) {

  }

  var_index_t left_idx;
  BinopType op;
  T right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_integral_type_v<T>)
struct Shift : Expr {
  using result_t = T;
  using src_t = T;
  // shift type changed when value is negative anyway
  using shift_t = u32;
  Shift(var_index_t idx, var_index_t left, ShiftType op, var_index_t right) :
      Expr(GetTID(), idx), left_idx(left), op(op), right_idx(right) {

  }

  var_index_t left_idx;
  ShiftType op;
  // right type is ALWAYS u32
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_integral_type_v<T>)
struct ShiftImm : Expr {
  using result_t = T;
  using src_t = T;
  ShiftImm(var_index_t idx, var_index_t left, ShiftType op, T right) :
      Expr(GetTID(), idx), left_idx(left), op(op), right(right) {

  }

  var_index_t left_idx;
  ShiftType op;
  T right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_arithmetic_ptr_type_v<T>)
struct Compare : Expr {
  using result_t = i32;
  using src_t = T;
  Compare(var_index_t idx, var_index_t left, CmpType op, var_index_t right) :
          Expr(GetTID(), idx), left_idx(left), op(op), right_idx(right) {

  }

  var_index_t left_idx;
  CmpType op;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_arithmetic_ptr_type_v<T>)
struct CompareImm : Expr {
  using result_t = i32;
  using src_t = T;
  CompareImm(var_index_t idx, var_index_t left, CmpType op, T right) :
          Expr(GetTID(), idx), left_idx(left), op(op), right(right) {

  }

  var_index_t left_idx;
  CmpType op;
  T right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
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
  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_arithmetic_ptr_type_v<T>)
struct BranchCompare : Branch {
  using src_t = T;

  BranchCompare(block_label_t tdest, block_label_t fdest, var_index_t left, CmpType op, var_index_t right) :
          Branch(Class::Branch, GetTID()), 
          tdest(tdest), fdest(fdest),
          left_idx(left), op(op), right_idx(right) {

  }

  block_label_t tdest;
  block_label_t fdest;

  var_index_t left_idx;
  CmpType op;
  var_index_t right_idx;

  std::vector<block_label_t> Destinations() const final {
    return {tdest, fdest};
  }
  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_arithmetic_ptr_type_v<T>)
struct BranchCompareImm : Branch {
  using src_t = T;

  BranchCompareImm(block_label_t tdest, block_label_t fdest, var_index_t left, CmpType op, T right) :
          Branch(Class::Branch, GetTID()),
          tdest(tdest), fdest(fdest),
          left_idx(left), op(op), right(right) {

  }

  block_label_t tdest;
  block_label_t fdest;
  
  var_index_t left_idx;
  CmpType op;
  T right;

  std::vector<block_label_t> Destinations() const final {
    return {tdest, fdest};
  }
  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_integral_type_v<T>)
struct AddToPointer : Expr {
  using result_t = Pointer;
  using offset_t = T;

  AddToPointer(var_index_t idx, var_index_t ptr, u64 stride, var_index_t right) :
      Expr(GetTID(), idx), ptr_idx(ptr), stride(stride), right_idx(right) {

  }

  var_index_t ptr_idx;
  u64 stride;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

struct AddToPointerImm : Expr {
  using result_t = Pointer;
  using offset_t = i64;
  
  AddToPointerImm(var_index_t idx, var_index_t ptr, u64 stride, i64 right) :
      Expr(GetTID(), idx), ptr_idx(ptr), stride(stride), right(right) {

  }

  var_index_t ptr_idx;
  u64 stride;
  offset_t right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
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

  std::string ToString() const final;
  void Emit(Backend& backend) final;
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

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
struct LoadLocal : Expr {
  using result_t = calyx_upcast_t<T>;
  LoadLocal(var_index_t idx, var_index_t loc_idx, i32 offset = 0) :
      Expr(GetTID(), idx), loc_idx(loc_idx), offset(offset) {

  }

  var_index_t loc_idx;
  i32 offset;  // struct fields

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

struct LoadLocalAddr : Expr {
  using result_t = Pointer;
  LoadLocalAddr(var_index_t idx, var_index_t loc_idx) :
          Expr(GetTID(), idx), loc_idx(loc_idx){

  }

  var_index_t loc_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
struct StoreLocal : Directive {
  using src_t = calyx_upcast_t<T>;

  StoreLocal(var_index_t loc_idx, var_index_t src, i32 offset = 0) :
      Directive(Class::Store, GetTID()), loc_idx(loc_idx), src(src), offset(offset) {

  }

  var_index_t loc_idx;
  var_index_t src;
  i32 offset;  // struct fields

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
struct LoadGlobal : Expr {
  using result_t = calyx_upcast_t<T>;
  LoadGlobal(var_index_t idx, std::string symbol, i32 offset = 0) :
      Expr(GetTID(), idx), symbol(std::move(symbol)), offset(offset) {

  }

  std::string symbol;
  i32 offset;  // struct fields

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

struct LoadGlobalAddr : Expr {
  using result_t = Pointer;
  LoadGlobalAddr(var_index_t idx, std::string symbol) :
          Expr(GetTID(), idx), symbol(std::move(symbol)){

  }

  std::string symbol;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
struct StoreGlobal : Directive {
  using src_t = calyx_upcast_t<T>;

  StoreGlobal(std::string symbol, var_index_t src, i32 offset = 0) :
      Directive(Class::Store, GetTID()), symbol(std::move(symbol)), src(src), offset(offset) {

  }

  std::string symbol;
  var_index_t src;
  i32 offset;  // struct fields

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

struct AllocateLocal : Directive {

  AllocateLocal(var_index_t loc_idx, u64 size) :
      Directive(Class::Stack, GetTID()), loc_idx(loc_idx), size(size) {

  }

  var_index_t loc_idx;
  u64 size;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

struct DeallocateLocal : Directive {

  DeallocateLocal(var_index_t loc_idx, u64 size) :
      Directive(Class::Stack, GetTID()), loc_idx(loc_idx), size(size) {

  }

  var_index_t loc_idx;
  u64 size;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
struct LoadFromPointer : Expr {
  using result_t = calyx_upcast_t<T>;
  
  LoadFromPointer(var_index_t idx, var_index_t ptr_idx, i32 offset = 0) :
      Expr(GetTID(), idx), ptr_idx(ptr_idx), offset(offset) {

  }

  var_index_t ptr_idx;
  i32 offset;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
struct StoreToPointer : Directive {
  using src_t = calyx_upcast_t<T>;

  StoreToPointer(var_index_t ptr_idx, var_index_t src, i32 offset = 0) :
      Directive(Class::Store, GetTID()), ptr_idx(ptr_idx), src(src), offset(offset) {

  }

  var_index_t ptr_idx;
  var_index_t src;
  i32 offset;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_type_v<T> || std::is_same_v<T, void>)
struct Return : Directive {
  using src_t = T;

  Return(var_index_t idx) :
    Directive(Class::Return, GetTID()), idx(idx) {

  }

  var_index_t idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_type_v<T> || std::is_same_v<T, void>)
struct Call : Directive {
  using result_t = T;

  Call(var_index_t idx, var_index_t fn_idx, arg_list_t args, arg_list_t var_args) :
      Directive(Class::Call, GetTID()), idx(idx), fn_idx(fn_idx), args(std::move(args)), var_args(std::move(var_args)) {

  }

  var_index_t idx;
  var_index_t fn_idx;
  arg_list_t args;
  arg_list_t var_args;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_type_v<T> || std::is_same_v<T, void>)
struct CallLabel : Directive {
  using result_t = T;

  CallLabel(var_index_t idx, std::string label, arg_list_t args, arg_list_t var_args) :
    Directive(Class::Call, GetTID()), idx(idx), label(std::move(label)), args(std::move(args)), var_args(std::move(var_args)) {

  }

  var_index_t idx;
  std::string label;
  arg_list_t args;
  arg_list_t var_args;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

struct ArgMakeLocal : Directive {

  ArgMakeLocal(Argument arg, var_index_t loc_idx) :
      Directive(Class::Stack, GetTID()), arg(arg), loc_idx(loc_idx) {

  }

  Argument arg;
  var_index_t loc_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
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
  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

}