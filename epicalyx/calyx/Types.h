#pragma once

#include "Default.h"
#include "Packs.h"
#include "CString.h"
#include "Stringify.h"
#include "CustomAssert.h"
#include "Decltype.h"
#include "swl/variant.hpp"
#include "Aligned.h"
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

struct Aggregate {
  u64 size;
  u32 align;
};

STRINGIFY_METHOD(Aggregate);

template<typename T>
struct Scalar {
  using type_t = T;
  T value;

  bool operator==(const Scalar<T>& other) const = default;
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

  bool operator==(const Operand<T>& other) const {
    return std::memcmp(this, &other, sizeof(*this)) == 0;
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
    Float, Double, Pointer, Aggregate
  };

  // arguments can never be aggregates,
  // converted to pointers
  Local(Type type, loc_index_t idx, std::optional<var_index_t>&& arg_idx = {}) : 
      type{type}, idx{idx}, non_aggregate{std::move(arg_idx)} {

  }

  static Local Aggregate(loc_index_t idx, calyx::Aggregate&& aggregate) {
    return Local(Type::Aggregate, idx, std::move(aggregate));
  }

  static Local Pointer(loc_index_t idx, u64 stride, std::optional<var_index_t>&& arg_idx = {}) {
    auto loc = Local(Type::Pointer, idx, std::move(arg_idx));
    loc.non_aggregate.stride = stride;
    return loc;
  }

  u64 Size() const {
    switch (type) {
      case Type::I8: case Type::U8: return 1;
      case Type::I16: case Type::U16: return 2;
      case Type::I32: case Type::U32: return 4;
      case Type::I64: case Type::U64: return 8;
      case Type::Pointer: return 8;
      case Type::Aggregate: return aggregate.size;
    }
  }

  Type type;
  loc_index_t idx;
  union {
    calyx::Aggregate aggregate;
    struct {
      std::optional<loc_index_t> arg_idx;
      u64 stride;
    } non_aggregate;
  };

private:

  Local(Type type, loc_index_t idx, calyx::Aggregate&& aggregate) : 
      type{type}, idx{idx}, aggregate{std::move(aggregate)} {
        
  }

};

struct ArgData {
  arg_list_t args;
  arg_list_t var_args;
};


struct LabelOffset {
  cotyl::CString label;
  i64 offset;
};


struct AggregateData {
  AggregateData(Aggregate&& agg_) : 
      agg{std::move(agg_)}, data{cotyl::make_ualigned<u8>(agg.align, agg.size)} { }
  AggregateData(u64 size, u32 align) : AggregateData(Aggregate{size, align}) { };

  Aggregate agg;
  cotyl::aligned_uptr<u8> data;
};

namespace detail {

using global_t = swl::variant<
   Scalar<i8>, Scalar<u8>, 
   Scalar<i16>, Scalar<u16>, 
   Scalar<i32>, Scalar<u32>, 
   Scalar<i64>, Scalar<u64>, 
   Scalar<float>, Scalar<double>, 
   Pointer, LabelOffset,
   AggregateData
>;

}

struct Global : detail::global_t {
  using detail::global_t::global_t;

};

STRINGIFY_METHOD(Global);

}

}