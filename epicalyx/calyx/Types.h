#pragma once

#include "Default.h"
#include "Packs.h"
#include "CString.h"
#include "Stringify.h"
#include "CustomAssert.h"
#include "Decltype.h"
#include "CalyxFwd.h"

#include <variant>
#include <string>
#include <optional>


namespace epi {

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

struct ArgData {
  arg_list_t args;
  arg_list_t var_args;
};

}

}