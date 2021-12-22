#pragma once

#include "Default.h"
#include "Is.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>


namespace epi::calyx {

struct Backend;

using var_index_t = u64;
using block_label_t = u64;

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
constexpr bool is_calyx_type_v = epi::cotyl::is_in_v<T, i32, u32, i64, u64, float, double, Struct, Pointer>;
template<typename T>
constexpr bool is_calyx_arithmetic_type_v = epi::cotyl::is_in_v<T, i32, u32, i64, u64, float, double>;
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

struct Local {
  enum class Location {
    Stack,  // if address is taken
    Register,
    Either,
  };

  var_index_t idx;
  Location loc;
  u64 size;
};

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
    UnconditionalBranch,
    Call,
    Return,
    Select,
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

template<typename T>
requires (is_calyx_type_v<T>)
struct Expr : Directive {
  
  Expr(size_t type_id, var_index_t idx) :
      Directive(Class::Expression, type_id),
    idx(idx) {

  }

  var_index_t idx;
};

struct Branch : Directive {

  Branch(Class cls, size_t type_id, block_label_t dest) :
      Directive(cls, type_id),
      dest(dest) {

  }

  block_label_t dest;
};

struct Program {
  using block_t = std::vector<pDirective>;
  using blocks_t = std::unordered_map<block_label_t, block_t>;

  Program() {
    blocks.emplace(0, block_t{});
  }

  // program code
  // block 0 is special
  blocks_t blocks{};

  // function symbols -> block ID
  std::unordered_map<std::string, calyx::block_label_t> functions{};

  // local label -> block ID
  // todo: move this back to the AST walker
  std::unordered_map<std::string, calyx::block_label_t> local_labels{};

  // string constants
  std::vector<std::string> strings{};

  // global variable sizes
  std::unordered_map<std::string, size_t> globals{};

  // global variable initializer
  // possible constant, possibly a label with some offset, possibly requires some code
  // for example, weird expressions like "long long k = 2 * (long long)test" are too hard to parse
  // where test is another symbol (e.g. int test(int);)
  // (try this on godbolt for example)
  // these require a block to run
  std::unordered_map<std::string, std::variant<std::vector<u8>, label_offset_t, block_label_t>> global_init{};
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
      Expr<calyx_upcast_t<To>>(GetTID(), idx), right_idx(right_idx) {

  }

  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_arithmetic_type_v<T>)
struct Binop : Expr<T> {

  Binop(var_index_t idx, var_index_t left, BinopType op, var_index_t right) :
      Expr<T>(GetTID(), idx), left_idx(left), op(op), right_idx(right) {

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
struct BinopImm : Expr<T> {

  BinopImm(var_index_t idx, var_index_t left, BinopType op, T right) :
      Expr<T>(GetTID(), idx), left_idx(left), op(op), right(right) {

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
struct Shift : Expr<T> {

  Shift(var_index_t idx, var_index_t left, ShiftType op, var_index_t right) :
      Expr<T>(GetTID(), idx), left_idx(left), op(op), right_idx(right) {

  }

  var_index_t left_idx;
  ShiftType op;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_integral_type_v<T>)
struct ShiftImm : Expr<T> {

  ShiftImm(var_index_t idx, var_index_t left, ShiftType op, T right) :
      Expr<T>(GetTID(), idx), left_idx(left), op(op), right(right) {

  }

  var_index_t left_idx;
  ShiftType op;
  T right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_type_v<T>)
struct Compare : Expr<i32> {

  Compare(var_index_t idx, var_index_t left, CmpType op, var_index_t right) :
          Expr<i32>(GetTID(), idx), left_idx(left), op(op), right_idx(right) {

  }

  var_index_t left_idx;
  CmpType op;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_type_v<T>)
struct CompareImm : Expr<i32> {

  CompareImm(var_index_t idx, var_index_t left, CmpType op, T right) :
          Expr<i32>(GetTID(), idx), left_idx(left), op(op), right(right) {

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
      Branch(Class::UnconditionalBranch, GetTID(), dest) {

  }

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_type_v<T>)
struct BranchCompare : Branch {

  BranchCompare(block_label_t dest, var_index_t left, CmpType op, var_index_t right) :
      Branch(Class::Branch, GetTID(), dest), left_idx(left), op(op), right_idx(right) {

  }

  var_index_t left_idx;
  CmpType op;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_type_v<T>)
struct BranchCompareImm : Branch {

  BranchCompareImm(block_label_t dest, var_index_t left, CmpType op, T right) :
    Branch(Class::Branch, GetTID(), dest), left_idx(left), op(op), right(right) {

  }

  var_index_t left_idx;
  CmpType op;
  T right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_integral_type_v<T>)
struct AddToPointer : Expr<Pointer> {

  AddToPointer(var_index_t idx, var_index_t ptr, PtrAddType op, u64 stride, var_index_t right) :
      Expr<Pointer>(GetTID(), idx), ptr_idx(ptr), op(op), stride(stride), right_idx(right) {

  }

  var_index_t ptr_idx;
  PtrAddType op;
  u64 stride;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

struct AddToPointerImm : Expr<Pointer> {

  AddToPointerImm(var_index_t idx, var_index_t ptr, u64 stride, i64 right) :
      Expr<Pointer>(GetTID(), idx), ptr_idx(ptr), stride(stride), right(right) {

  }

  var_index_t ptr_idx;
  u64 stride;
  i64 right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_type_v<T>)
struct Imm : Expr<T> {

  Imm(var_index_t idx, T value) :
      Expr<T>(GetTID(), idx), value(value) {

  }

  T value;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_arithmetic_type_v<T>)
struct Unop : Expr<T> {

  Unop(var_index_t idx, UnopType op, var_index_t right) :
      Expr<T>(GetTID(), idx), op(op), right_idx(right) {

  }

  UnopType op;
  var_index_t right_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
struct LoadLocal : Expr<calyx_upcast_t<T>> {

  LoadLocal(var_index_t idx, var_index_t loc_idx, i32 offset = 0) :
      Expr<calyx_upcast_t<T>>(GetTID(), idx), loc_idx(loc_idx), offset(offset) {

  }

  var_index_t loc_idx;
  i32 offset;  // struct fields

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

struct LoadLocalAddr : Expr<Pointer> {

  LoadLocalAddr(var_index_t idx, var_index_t loc_idx) :
          Expr<Pointer>(GetTID(), idx), loc_idx(loc_idx){

  }

  var_index_t loc_idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
struct StoreLocal : Directive {

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
struct LoadGlobal : Expr<calyx_upcast_t<T>> {

  LoadGlobal(var_index_t idx, std::string symbol, i32 offset = 0) :
      Expr<calyx_upcast_t<T>>(GetTID(), idx), symbol(std::move(symbol)), offset(offset) {

  }

  std::string symbol;
  i32 offset;  // struct fields

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

struct LoadGlobalAddr : Expr<Pointer> {

  LoadGlobalAddr(var_index_t idx, std::string symbol) :
          Expr<Pointer>(GetTID(), idx), symbol(std::move(symbol)){

  }

  std::string symbol;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
struct StoreGlobal : Directive {

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
struct LoadFromPointer : Expr<calyx_upcast_t<T>> {

  LoadFromPointer(var_index_t idx, var_index_t ptr_idx, i32 offset = 0) :
      Expr<calyx_upcast_t<T>>(GetTID(), idx), ptr_idx(ptr_idx), offset(offset) {

  }

  var_index_t ptr_idx;
  i32 offset;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
struct StoreToPointer : Directive {

  StoreToPointer(var_index_t ptr_idx, var_index_t src, i32 offset = 0) :
      Directive(Class::Store, GetTID()), src(src), ptr_idx(ptr_idx), offset(offset) {

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

  Return(var_index_t idx) :
    Directive(Class::Return, GetTID()), idx(idx) {

  }

  var_index_t idx;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
struct Call : Directive {

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
struct CallLabel : Directive {

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

struct Select : Directive {

  Select(var_index_t idx) :
      Directive(Class::Select, GetTID()), idx(idx) {

  }

  var_index_t idx;
  std::unordered_map<i64, block_label_t> table{};
  block_label_t _default = 0;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

}