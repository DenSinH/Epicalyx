#pragma once

#include "Default.h"
#include "Packs.h"
#include "CString.h"
#include "Stringify.h"
#include "CustomAssert.h"

#include <variant>
#include <string>
#include <optional>


namespace epi {

namespace calyx {

struct Function;

}

using var_index_t = u32;
using block_label_t = u32;
using func_pos_t = std::pair<block_label_t, int>;

struct program_pos_t {
  const calyx::Function* func;
  func_pos_t pos;
};

struct label_offset_t {
  cotyl::CString label;
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

using calyx_small_types = cotyl::pack<i8, u8, i16, u16>;
using calyx_integral_types = cotyl::pack<i32, u32, i64, u64>;
using calyx_arithmetic_types = cotyl::flatten_pack<calyx_integral_types, float, double>;
using calyx_arithmetic_ptr_types = cotyl::flatten_pack<calyx_arithmetic_types, calyx::Pointer>;
using calyx_types = cotyl::flatten_pack<calyx_arithmetic_ptr_types, calyx::Struct>;
using calyx_return_types = cotyl::flatten_pack<calyx_types, void>;
using calyx_memory_types = cotyl::flatten_pack<calyx_types, calyx_small_types>;

template<typename T>
constexpr bool is_calyx_small_type_v = cotyl::pack_contains_v<T, calyx_small_types>;
template<typename T>
constexpr bool is_calyx_integral_type_v = cotyl::pack_contains_v<T, calyx_integral_types>;
template<typename T>
constexpr bool is_calyx_arithmetic_type_v = cotyl::pack_contains_v<T, calyx_arithmetic_types>;
template<typename T>
constexpr bool is_calyx_arithmetic_ptr_type_v = cotyl::pack_contains_v<T, calyx_arithmetic_ptr_types>;
template<typename T>
constexpr bool is_calyx_type_v = cotyl::pack_contains_v<T, calyx_types>;

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
  Operand(S var_idx) : is_var{true}, var{(var_index_t)var_idx} { }

  Operand(const Var& var) : is_var{true}, var{var} { }
  Operand(Var&& var) : is_var{true}, var{std::move(var)} { }
  Operand(const Imm& imm) : is_var{false}, imm{imm} { }
  Operand(Imm&& imm) : is_var{false}, imm{std::move(imm)} { }

  bool IsImm() const { return !is_var; }
  const T& GetImm() const {
    cotyl::Assert(IsImm(), "Invalid operand access");
    return imm.value; 
  }

  T& GetImm() { 
    cotyl::Assert(IsImm(), "Invalid operand access");
    return imm.value; 
  }
  
  bool IsVar() const { return is_var; }
  const var_index_t& GetVar() const { 
    cotyl::Assert(IsVar(), "Invalid operand access");
    return var.var_idx; 
  }

  var_index_t& GetVar() { 
    cotyl::Assert(IsVar(), "Invalid operand access");
    return var.var_idx;
  }

private:
  bool is_var;
  union {
    Var var;
    Imm imm;
  };
};

struct Local {

  enum class Type : u32 {
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

// IR var idx and Argument
using arg_list_t = cotyl::vector<std::pair<var_index_t, Argument>>;

struct ArgData {
  arg_list_t args;
  arg_list_t var_args;
};

using global_t = std::variant<i8, u8, i16, u16, i32, u32, i64, u64, float, double, Pointer, label_offset_t>;

}

}