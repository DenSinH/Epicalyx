#pragma once

#include "Default.h"
#include "Packs.h"
#include "CString.h"
#include "Stringify.h"
#include "CustomAssert.h"
#include "Decltype.h"
#include "swl/variant.hpp"
#include "CalyxFwd.h"


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
struct Scalar {
  using type_t = T;
  T value;
};

template<typename T>
requires (is_calyx_type_v<T>)
struct Operand {
  struct Var {
    var_index_t var_idx;
  };

  // ALWAYS instantiate as var index
  template<typename S>
  requires (std::is_integral_v<S>)
  Operand(S var_idx) : is_var{true}, var{(var_index_t)var_idx} { }

  Operand(const Var& var) : is_var{true}, var{var} { }
  Operand(Var&& var) : is_var{true}, var{std::move(var)} { }
  Operand(const Scalar<T>& imm) : is_var{false}, imm{imm} { }
  Operand(Scalar<T>&& imm) : is_var{false}, imm{std::move(imm)} { }

  bool IsScalar() const { return !is_var; }
  const T& GetScalar() const {
    cotyl::Assert(IsScalar(), "Invalid operand access");
    return imm.value; 
  }

  T& GetScalar() { 
    cotyl::Assert(IsScalar(), "Invalid operand access");
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
    Scalar<T> imm;
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


struct LabelOffset {
  cotyl::CString label;
  i64 offset;
};

namespace detail {

using global_t = swl::variant<
   Scalar<i8>, Scalar<u8>, 
   Scalar<i16>, Scalar<u16>, 
   Scalar<i32>, Scalar<u32>, 
   Scalar<i64>, Scalar<u64>, 
   Scalar<float>, Scalar<double>, 
   Pointer, LabelOffset
>;

}

struct Global : detail::global_t {
  using detail::global_t::global_t;

};

}

}