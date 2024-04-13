#pragma once

#include "Default.h"
#include "Is.h"
#include "Containers.h"
#include "Stringify.h"

#include <memory>
#include <string>
#include <vector>
#include <variant>
#include <optional>
#include <bit>

namespace epi {

namespace calyx {

struct Backend;
struct Function;

}

using var_index_t = u64;
using block_label_t = u64;
using func_pos_t = std::pair<block_label_t, int>;

struct program_pos_t {
  const calyx::Function* func;
  func_pos_t pos;
};

struct label_offset_t {
  std::string label;
  i64 offset;
};

namespace calyx {

using ::epi::stringify;

struct Pointer {
  Pointer() : value(0) { }
  Pointer(i64 value) : value(value) { }

  i64 value;
};

STRINGIFY_METHOD(Pointer);

static_assert(sizeof(Pointer) == sizeof(i64));

struct Struct { };

STRINGIFY_METHOD(Struct);

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

template<typename T>
requires (is_calyx_type_v<T>)
struct Operand {
  struct Var {
    var_index_t var_idx;
  };

  struct Imm {
    T value;
  };

  // ALWAYS instantiate as var index
  template<typename S>
  requires (std::is_integral_v<S>)
  Operand(S var_idx) : value{Var{(var_index_t)var_idx}} { }

  Operand(const Var& var) : value{var} { }
  Operand(Var&& var) : value{std::move(var)} { }
  Operand(const Imm& imm) : value{imm} { }
  Operand(Imm&& imm) : value{std::move(imm)} { }

  bool IsImm() const { return std::holds_alternative<Imm>(value); }
  const T& GetImm() const { return std::get<Imm>(value).value; }
  T& GetImm() { return std::get<Imm>(value).value; }
  
  bool IsVar() const { return std::holds_alternative<Var>(value); }
  const var_index_t& GetVar() const { return std::get<Var>(value).var_idx; }
  var_index_t& GetVar() { return std::get<Var>(value).var_idx; }

private:
  std::variant<Var, Imm> value;
};

struct Local {

  enum class Type {
    I8, U8, I16, U16, 
    I32, U32, I64, U64,
    Float, Double, Pointer, Struct
  };

  Local(Type type, var_index_t idx, size_t size, std::optional<var_index_t>&& arg_idx) : 
      type{type}, idx{idx}, size{size}, arg_idx{std::move(arg_idx)} {

  }

  Local(Type type, var_index_t idx, size_t size) : 
      Local{type, idx, size, {}} {

  }

  Type type;
  var_index_t idx;
  size_t size;
  std::optional<var_index_t> arg_idx{};
};

struct Argument {

  Argument() = default;
  
  Argument(Local::Type type, var_index_t arg_idx, bool variadic = false) :
          type(type), arg_idx(arg_idx), variadic(variadic), stride(0) {

  }

  Argument(Local::Type type, var_index_t arg_idx, u64 stride, bool variadic = false) :
          type(type), arg_idx(arg_idx), variadic(variadic), stride(stride) {

  }

  Local::Type type;
  var_index_t arg_idx;
  bool variadic;
  union {
    u64 stride;  // for pointers
    u64 size;    // for structs
  };
};

struct Directive;
using pDirective = std::unique_ptr<Directive>;
using block_t = std::vector<pDirective>;
using global_t = std::variant<i8, u8, i16, u16, i32, u32, i64, u64, float, double, Pointer, label_offset_t>;

struct Function {
  static constexpr block_label_t Entry = 1;

  Function(const std::string& symbol) : symbol{symbol} { }

  std::string symbol;
  // program code
  // block 0 is special
  cotyl::unordered_map<block_label_t, block_t> blocks{};

  cotyl::unordered_map<var_index_t, Local> locals{};  
  
  size_t Hash() const;
};

struct Program {
  // function symbols -> entrypoint block ID
  cotyl::unordered_map<std::string, Function> functions{};

  // string constants
  std::vector<std::string> strings{};

  // global variable sizes
  cotyl::unordered_map<std::string, global_t> globals{};

  size_t Hash() const;
};

void VisualizeProgram(const Program& program, const std::string& filename);
void PrintProgram(const Program& program);

// IR var idx and Argument
using arg_list_t = std::vector<std::pair<var_index_t, Argument>>;

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
struct Store : Directive {
  using src_t = calyx_upcast_t<T>;

  Store(size_t type_id, Operand<src_t> src) : 
      Directive(Class::Store, type_id),
      src{src} {

  }

  Operand<src_t> src;
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
struct Cast final : Expr {
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
struct Binop final : Expr {
  using result_t = T;
  using src_t = T;
  Binop(var_index_t idx, var_index_t left, BinopType op, Operand<T> right) :
      Expr(GetTID(), idx), left_idx(left), op(op), right(right) {

  }

  var_index_t left_idx;
  BinopType op;
  Operand<T> right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_integral_type_v<T>)
struct Shift final : Expr {
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

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_arithmetic_ptr_type_v<T>)
struct Compare final : Expr {
  using result_t = i32;
  using src_t = T;
  Compare(var_index_t idx, var_index_t left, CmpType op, Operand<T> right) :
          Expr(GetTID(), idx), left_idx(left), op(op), right(right) {

  }

  var_index_t left_idx;
  CmpType op;
  Operand<T> right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

struct UnconditionalBranch final : Branch {
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
struct BranchCompare final : Branch {
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
  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_integral_type_v<T>)
struct AddToPointer final : Expr {
  using result_t = Pointer;
  using offset_t = T;

  AddToPointer(var_index_t idx, Operand<Pointer> ptr, u64 stride, Operand<T> right) :
      Expr(GetTID(), idx), ptr(ptr), stride(stride), right(right) {

  }

  Operand<Pointer> ptr;
  u64 stride;
  Operand<offset_t> right;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_type_v<T>)
struct Imm final : Expr {
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
struct Unop final : Expr {
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
struct LoadLocal final : Expr {
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

struct LoadLocalAddr final : Expr {
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
struct StoreLocal final : public Store<T> {
  using src_t = Store<T>::src_t;

  StoreLocal(var_index_t loc_idx, Operand<src_t> src, i32 offset = 0) :
      Store<T>(GetTID(), src), 
      loc_idx(loc_idx), 
      offset(offset) {

  }

  var_index_t loc_idx;
  i32 offset;  // struct fields

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
struct LoadGlobal final : Expr {
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

struct LoadGlobalAddr final : Expr {
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
struct StoreGlobal final : public Store<T> {
  using src_t = Store<T>::src_t;

  StoreGlobal(std::string symbol, Operand<src_t> src, i32 offset = 0) :
      Store<T>(GetTID(), src),
      symbol(std::move(symbol)), 
      offset(offset) {

  }

  std::string symbol;
  i32 offset;  // struct fields

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};


template<typename T>
struct LoadFromPointer final : Expr {
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
struct StoreToPointer final : public Store<T> {
  using src_t = Store<T>::src_t;

  StoreToPointer(var_index_t ptr_idx, Operand<src_t> src, i32 offset = 0) :
      Store<T>(GetTID(), src), 
      ptr_idx(ptr_idx), 
      offset(offset) {

  }

  var_index_t ptr_idx;
  i32 offset;

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_type_v<T> || std::is_same_v<T, void>)
struct Return final : Directive {
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

  std::string ToString() const final;
  void Emit(Backend& backend) final;
  static constexpr size_t GetTID() { return std::bit_cast<size_t>(&GetTID); }
};

template<typename T>
requires (is_calyx_type_v<T> || std::is_same_v<T, void>)
struct Call final : Directive {
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

struct Select final : Branch {
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

}  // namespace calyx

}  // namespace epi