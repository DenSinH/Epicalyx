#include "Interpreter.h"
#include "CustomAssert.h"
#include "Exceptions.h"
#include "Stringify.h"
#include "Format.h"
#include "Decltype.h"

#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <limits>


namespace epi::calyx {

struct InterpreterError : cotyl::Exception {
  InterpreterError(std::string&& message) : 
      Exception("Interpreter Error", std::move(message)) { }
};

template<typename T>
struct scalar_or_pointer {
  using type = Scalar<T>;
};

template<>
struct scalar_or_pointer<Pointer> {
  using type = Pointer;
};

template<typename T>
using scalar_or_pointer_t = scalar_or_pointer<T>::type;

using ::epi::stringify;

static STRINGIFY_METHOD(Interpreter::var_real_t) {
  return swl::visit<std::string>(
    swl::overloaded{
      [](const epi::calyx::Pointer& var) {
        return cotyl::Format("pointerid", var.value);
      },
      []<typename T>(const epi::calyx::Scalar<T>& var) {
        return cotyl::Format("%d (%d)", var.value, 8 * sizeof(T));
      },
      swl::exhaustive
    }, 
    value
  );
}

void Interpreter::DumpVars() const {
  for (const auto& layer : vars) {
    std::cout << "Layer (" << layer.size() << " vars)" << std::endl;
    for (const auto& [var_idx, var] : layer) {
      std::cout << "  " << var_idx << " = " << stringify(var) << std::endl;
    }
  }
}

void Interpreter::InterpretGlobalInitializer(Global& dest, Function&& func) {
  calyx::ArgData no_args{};
  call_stack.emplace(program_counter_t{nullptr, {0, 0}}, -1, &no_args);
  EnterFunction(&func);

  while (pos.pos.first) {
    auto& directive = pos.func->blocks.at(pos.pos.first).at(pos.pos.second);
    pos.pos.second++;
    Emit(directive);
    // calls CANNOT happen
    cotyl::Assert(!called, "Call invalid in global ininitializer");
  }

  // todo: visit global type and get var based on that
  // that is what caused the bad global types
  const auto& result = vars.Get(-1);
  swl::visit(
    swl::overloaded{
      [&](Pointer& glob) {
        const auto& ptr = swl::get<Pointer>(result);
        auto real_pointer = ReadPointer(ptr.value);
        if (swl::holds_alternative<i64>(real_pointer)) {
          glob = Pointer{swl::get<i64>(real_pointer)};
        }
        else {
          dest.emplace<LabelOffset>(swl::get<LabelOffset>(real_pointer));
        }
      },
      [&](LabelOffset& glob) {
        auto&& ptr = swl::get<Pointer>(result);
        auto&& loffs = swl::get<LabelOffset>(ReadPointer(ptr.value));
        glob.label = std::move(loffs.label);
        glob.offset = loffs.offset;
      },
      [&]<typename T>(Scalar<T>& glob) {
        glob.value = swl::get<Scalar<typename StoreGlobal<T>::src_t>>(result).value;
      },
      [](const auto&) {
        throw cotyl::UnimplementedException("Global struct initializer");
      }
    },
    dest
  );
}

void Interpreter::Emit(const AnyDirective& dir) {
  dir.visit<void>([&](const auto& d) {
    // std::cout << stringify(d) << std::endl;
    Emit(d); 
  });
}

i32 Interpreter::Interpret(Program&& program) {
  for (auto& [symbol, global] : program.globals) {
    swl::visit(
      swl::overloaded{
        // we have to handle label offsets differently, since
        // instructions do not care about LabelOffset types, only 
        // Pointer types
        [&](LabelOffset&& glob) {
          Pointer ptr = MakePointer(std::move(glob));
          globals.emplace(std::move(symbol), std::move(ptr));
        },
        [&](Pointer&& glob) {
          Pointer ptr = MakePointer(std::move(glob.value));
          globals.emplace(std::move(symbol), std::move(ptr));
        },
        // move aggregates
        [&](AggregateData&& glob) {
          globals.emplace(std::move(symbol), std::move(glob));
        },
        // any other type can just be passed straight off
        [&](auto& glob) {
          globals.emplace(std::move(symbol), std::move(glob));
        }
      },
      global
    );
  }

  auto argc = Scalar<i32>{1};
  auto argv = MakePointer(0);
  static constexpr var_index_t argc_idx = 1;
  static constexpr var_index_t argv_idx = 2;
  vars.Set(argc_idx, argc);
  vars.Set(argv_idx, argv);
  calyx::ArgData main_args{
    .args={
      {argc_idx, calyx::Local{calyx::Local::Type::I32, argc_idx, 0}},
      {argv_idx, calyx::Local::Pointer(argv_idx, sizeof(u64), 1)},
    }
  };
  call_stack.emplace(program_counter_t{nullptr, {0, 0}}, -1, &main_args);
  EnterFunction(&program.functions.at(cotyl::CString("main")));
  returned.reset();

  while (!returned.has_value()) {
    const auto& directive = pos.func->blocks.at(pos.pos.first).at(pos.pos.second);
    pos.pos.second++;
    Emit(directive);
    if (called.has_value()) {
      if (!program.functions.contains(called.value())) {
        throw cotyl::FormatExcept<InterpreterError>("Undefined function: %s", called.value().c_str());
      }
      EnterFunction(&program.functions.at(std::move(called.value())));
      called.reset();
    } 
  }

  if (!returned.has_value()) {
    throw InterpreterError("Program 'main' function did not return a value");
  }
  return returned.value();
}

void Interpreter::LoadArg(const calyx::Local& loc) {
  auto& [_, __, args] = call_stack.top();

  // locals have already been allocated on function entry
  const auto stack_loc = locals.Get(loc.idx).first;
  const auto arg_idx = loc.non_aggregate.arg_idx.value();
  switch (loc.type) {
    case Local::Type::I8: {
      i8 value = swl::get<Scalar<i32>>(vars.Get(args->args[arg_idx].first)).value;
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Local::Type::U8: {
      u8 value = swl::get<Scalar<u32>>(vars.Get(args->args[arg_idx].first)).value;
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Local::Type::I16: {
      i16 value = swl::get<Scalar<i32>>(vars.Get(args->args[arg_idx].first)).value;
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Local::Type::U16: {
      u16 value = swl::get<Scalar<u32>>(vars.Get(args->args[arg_idx].first)).value;
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Local::Type::I32: {
      i32 value = swl::get<Scalar<i32>>(vars.Get(args->args[arg_idx].first)).value;
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Local::Type::U32: {
      u32 value = swl::get<Scalar<u32>>(vars.Get(args->args[arg_idx].first)).value;
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Local::Type::I64: {
      i64 value = swl::get<Scalar<i64>>(vars.Get(args->args[arg_idx].first)).value;
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Local::Type::U64: {
      u64 value = swl::get<Scalar<u64>>(vars.Get(args->args[arg_idx].first)).value;
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Local::Type::Float: {
      float value = swl::get<Scalar<float>>(vars.Get(args->args[arg_idx].first)).value;
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Local::Type::Double: {
      double value = swl::get<Scalar<double>>(vars.Get(args->args[arg_idx].first)).value;
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Local::Type::Pointer: {
      Pointer value = swl::get<Pointer>(vars.Get(args->args[arg_idx].first));
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Local::Type::Aggregate: {
      throw cotyl::UnreachableException();
    }
  }
}

void Interpreter::EnterFunction(const Function* function) {
  pos.func = function;
  pos.pos.first = Function::Entry;
  pos.pos.second = 0;

  vars.NewLayer();

  // allocate locals
  locals.NewLayer();
  for (const auto& [loc_idx, local] : pos.func->locals) {
    locals.Set(loc_idx, std::make_pair(stack.size(), local.Size()));
    stack.resize(stack.size() + local.Size());

    if (local.non_aggregate.arg_idx.has_value()) {
      LoadArg(local);
    }
  }
}

template<typename To, typename From>
void Interpreter::Emit(const Cast<To, From>& op) {
//  cotyl::Assert(!vars.Top().contains(op.idx), stringify(op));
  using result_t = calyx_op_type(op)::result_t;
  using src_t = calyx_op_type(op)::src_t;
  if constexpr(std::is_same_v<To, Pointer>) {
    To value;
    From from = swl::get<scalar_or_pointer_t<From>>(vars.Get(op.right_idx)).value;
    if constexpr(std::is_same_v<From, Pointer>) {
      vars.Set(op.idx, MakePointer(ReadPointer(from.value)));
    }
    else {
      vars.Set(op.idx, MakePointer((i64)from));
    }
  }
  else if constexpr(std::is_same_v<From, Pointer>) {
    // we know that To is not a pointer type
    result_t value;
    src_t from = swl::get<src_t>(vars.Get(op.right_idx));

    // assume we don't get label offsets here, otherwise we read garbage anyway
    auto pval = ReadPointer(from.value);
    if (swl::holds_alternative<i64>(pval)) {
      value = swl::get<i64>(pval);
    }
    else {
      throw InterpreterError("Cast from non-direct pointer value");
    }
    vars.Set(op.idx, Scalar<result_t>{value});
  }
  else {
    result_t value;
    auto from = swl::get<Scalar<src_t>>(vars.Get(op.right_idx)).value;
    if constexpr(std::is_floating_point_v<src_t>) {
      if constexpr(std::is_floating_point_v<result_t>) {
        value = (result_t)from;
      }
      else {
        // out of bounds is UB anyway...
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-const-int-float-conversion"
        value = std::clamp<src_t>(
          from,
          std::numeric_limits<result_t>::min(),
          std::numeric_limits<result_t>::max()
        );
#pragma clang diagnostic pop
      }
    }
    else {
      value = from;
    }
    vars.Set(op.idx, Scalar<result_t>{(result_t)value});
  }
}

template<typename T>
void Interpreter::Emit(const LoadLocal<T>& op) {
  // works the same for pointers  
  using result_t = calyx_op_type(op)::result_t;
  T value;
  memcpy(&value, &stack[locals.Get(op.loc_idx).first], sizeof(T));
  vars.Set(op.idx, scalar_or_pointer_t<result_t>{(result_t)value});
}

void Interpreter::Emit(const LoadLocalAddr& op) {
  vars.Set(op.idx, MakePointer(locals.Get(op.loc_idx).first));
}

template<typename T>
void Interpreter::Emit(const StoreLocal<T>& op) {
  // works the same for pointers
  using src_t = calyx_op_type(op)::src_t;
  T value;
  if (op.src.IsVar()) {
    value = (T)swl::get<scalar_or_pointer_t<src_t>>(vars.Get(op.src.GetVar())).value;
  }
  else {
    value = (T)op.src.GetScalar();
  }
  memcpy(&stack[locals.Get(op.loc_idx).first], &value, sizeof(T));
}

template<typename T>
void Interpreter::Emit(const LoadGlobal<T>& op) {
  using result_t = calyx_op_type(op)::result_t;
  cotyl::Assert(swl::holds_alternative<scalar_or_pointer_t<T>>(globals.at(op.symbol)));
  result_t value = (T)swl::get<scalar_or_pointer_t<T>>(globals.at(op.symbol)).value;
  vars.Set(op.idx, scalar_or_pointer_t<result_t>{std::move(value)});
}

void Interpreter::Emit(const LoadGlobalAddr& op) {
  if (globals.contains(op.symbol)) {
    const auto& glob = globals.at(op.symbol);
    if (swl::holds_alternative<AggregateData>(glob)) {
      vars.Set(op.idx, MakePointer(swl::get<AggregateData>(glob).data.get()));
    }
  }
  vars.Set(op.idx, MakePointer(LabelOffset{cotyl::CString(op.symbol), 0}));
}

template<typename T>
void Interpreter::Emit(const StoreGlobal<T>& op) {
  using src_t = calyx_op_type(op)::src_t;
  cotyl::Assert(swl::holds_alternative<scalar_or_pointer_t<T>>(globals.at(op.symbol)));
  T value;
  if (op.src.IsVar()) {
    value = (T)swl::get<scalar_or_pointer_t<src_t>>(vars.Get(op.src.GetVar())).value;
  }
  else {
    value = (T)op.src.GetScalar();
  }
  globals.at(op.symbol).template emplace<scalar_or_pointer_t<T>>(std::move(value));
}

template<typename T>
void Interpreter::Emit(const LoadFromPointer<T>& op) {
  auto pointer = ReadPointer(swl::get<Pointer>(vars.Get(op.ptr_idx)).value);
  T value;
  swl::visit(
    swl::overloaded{
      [&](const i64& pval) {
        cotyl::Assert(pval >= 0);
        memcpy(&value, &stack[pval] + op.offset, sizeof(T));
      },
      [&](const LabelOffset& pval) {
        if (!globals.contains(pval.label)) {
          throw InterpreterError("Invalid symbol load");
        }

        const auto& glob = globals.at(pval.label);
        if (swl::holds_alternative<AggregateData>(glob)) {
          const auto& agg = swl::get<AggregateData>(glob);
          if (op.offset + sizeof(T) >= agg.agg.size) {
            throw InterpreterError("Out of bounds global aggregate load");
          }
          std::memcpy(&value, swl::get<AggregateData>(glob).data.get() + pval.offset + op.offset, sizeof(T));
        }
        else if (pval.offset != 0) {
          throw InterpreterError("Partial global data load");
        }
        else if (!swl::holds_alternative<scalar_or_pointer_t<T>>(glob)) {
          throw InterpreterError("Invalid aliased global data load");
        }
        else {
          value = swl::get<scalar_or_pointer_t<T>>(glob).value;
        }
      },
      [&](u8* const& agg) {
        memcpy(&value, agg + op.offset, sizeof(T));
      },
      swl::exhaustive
    },
    pointer
  );
  using result_t = calyx_op_type(op)::result_t;
  vars.Set(op.idx, scalar_or_pointer_t<result_t>{(result_t)value});
}

template<typename T>
void Interpreter::Emit(const StoreToPointer<T>& op) {
  auto pointer = ReadPointer(swl::get<Pointer>(vars.Get(op.ptr_idx)).value);
  using src_t = calyx_op_type(op)::src_t;
  T value;
  if (op.src.IsVar()) {
    value = (T)swl::get<scalar_or_pointer_t<src_t>>(vars.Get(op.src.GetVar())).value;
  }
  else {
    value = (T)op.src.GetScalar();
  }

  swl::visit(
    swl::overloaded{
      [&](const i64& pval) {
        cotyl::Assert(pval >= 0);
        memcpy(&stack[pval + op.offset], &value, sizeof(T));
      },
      [&](const LabelOffset& pval) {
        if (pval.offset != 0) {
          throw InterpreterError("Partial global data store");
        }

        if (!globals.contains(pval.label)) {
          throw InterpreterError("Invalid symbol store");
        }
        auto& glob = globals.at(pval.label);
        if (!swl::holds_alternative<scalar_or_pointer_t<T>>(glob)) {
          throw InterpreterError("Invalid aliased global data store");
        }
        glob.template emplace<scalar_or_pointer_t<T>>(std::move(value));
      },
      [&](u8* const& agg) {
        memcpy(agg + op.offset, &value, sizeof(T));
      },
      swl::exhaustive
    },
    pointer
  );
}

template<typename T>
void Interpreter::Emit(const Call<T>& op) {
  call_stack.emplace(pos, op.idx, op.args.get());
  auto pointer = ReadPointer(swl::get<Pointer>(vars.Get(op.fn_idx)).value);
  const Function* func;
  if (swl::holds_alternative<i64>(pointer) || swl::holds_alternative<u8*>(pointer)) {
    // pos.pos.first = swl::get<i64>(pointer);
    throw cotyl::UnimplementedException("Interpreter call pointer value");
  }
  else {
    const auto& pval = swl::get<LabelOffset>(pointer);
    called = cotyl::CString(pval.label);
    cotyl::Assert(pval.offset == 0, "Cannot jump to offset label in call");
  }
}

template<typename T>
void Interpreter::Emit(const CallLabel<T>& op) {
  call_stack.emplace(pos, op.idx, op.args.get());
  called = cotyl::CString(op.label);
}

template<typename T>
void Interpreter::Emit(const Return<T>& op) {
  auto top_vars = vars.Top();
  auto top_locals = locals.Top();
  locals.PopLayer();
  vars.PopLayer();

  auto [_pos, return_to, _] = call_stack.top();
  call_stack.pop();
  if (pos.func->symbol.str() == "main") {
    // return from main
    if constexpr(std::is_same_v<T, i32>) {
      if (op.val.IsVar()) {
        returned = swl::get<Scalar<i32>>(top_vars.at(op.val.GetVar())).value;
      }
      else {
        returned = op.val.GetScalar();
      }
    }
    else {
      throw InterpreterError("Invalid return type from 'main' symbol");
    }
  }
  else {
    pos = _pos;
    if constexpr(!std::is_same_v<T, void>) {
      if (op.val.IsVar()) vars.Set(return_to, top_vars.at(op.val.GetVar()));
      else vars.Set(return_to, scalar_or_pointer_t<T>{op.val.GetScalar()});
    }
  }

  for (const auto& [idx, local] : top_locals) {
    stack.resize(stack.size() - local.second);
  }
}

template<typename T>
void Interpreter::Emit(const Imm<T>& op) {
  // cotyl::Assert(!vars.Top().contains(op.idx), stringify(op));
  vars.Set(op.idx, scalar_or_pointer_t<T>{op.value});
}

template<typename T>
void Interpreter::Emit(const Unop<T>& op) {
  // cotyl::Assert(!vars.Top().contains(op.idx), stringify(op));
  T right = swl::get<Scalar<T>>(vars.Get(op.right_idx)).value;
  switch (op.op) {
    case UnopType::Neg:
      vars.Set(op.idx, Scalar<T>{(T)-right}); break;
    case UnopType::BinNot:
      if constexpr(std::is_integral_v<T>) {
        vars.Set(op.idx, Scalar<T>{(T)~right}); break;
      }
      else {
        throw InterpreterError("floating point operand for binary not");
      }
  }
}

template<typename T>
void Interpreter::Emit(const Binop<T>& op) {
  // cotyl::Assert(!vars.Top().contains(op.idx), stringify(op));
  T left = swl::get<Scalar<T>>(vars.Get(op.left_idx)).value;
  T right;
  if (op.right.IsVar()) {
    right = swl::get<Scalar<T>>(vars.Get(op.right.GetVar())).value;
  }
  else {
    right = op.right.GetScalar();
  }

  T result;
  switch (op.op) {
    case BinopType::Add: result = left + right; break;
    case BinopType::Sub: result = left - right; break;
    case BinopType::Mul: result = left * right; break;
    case BinopType::Div: result = left / right; break;
    case BinopType::Mod: {
      if constexpr(std::is_integral_v<T>) {
        result = left % right;
        break;
      }
      else {
        throw InterpreterError("Float operands for mod expression");
      }
    }
    case BinopType::BinAnd:{
      if constexpr(std::is_integral_v<T>) {
        result = left & right;
        break;
      }
      else {
        throw InterpreterError("Float operands for bin and expression");
      }
    }
    case BinopType::BinOr: {
      if constexpr(std::is_integral_v<T>) {
        result = left | right;
        break;
      }
      else {
        throw InterpreterError("Float operands for bin or expression");
      }
    }
    case BinopType::BinXor:{
      if constexpr(std::is_integral_v<T>) {
        result = left ^ right;
        break;
      }
      else {
        throw InterpreterError("Float operands for bin xor expression");
      }
    }
  }
  vars.Set(op.idx, Scalar<T>{result});
}

template<typename T>
void Interpreter::Emit(const Shift<T>& op) {
  // cotyl::Assert(!vars.Top().contains(op.idx), stringify(op));
  T left;
  calyx_op_type(op)::shift_t right;
  if (op.left.IsVar()) {
    left = swl::get<Scalar<T>>(vars.Get(op.left.GetVar())).value;
  }
  else {
    left = op.left.GetScalar();
  }
  if (op.right.IsVar()) {
    right = swl::get<Scalar<calyx_op_type(op)::shift_t>>(vars.Get(op.right.GetVar())).value;
  }
  else {
    right = op.right.GetScalar();
  }
  
  switch (op.op) {
    case calyx::ShiftType::Left: {
      left <<= right;
      break;
    }
    case calyx::ShiftType::Right: {
      left >>= right;
      break;
    }
  }
  vars.Set(op.idx, Scalar<T>{left});
}

template<typename T>
void Interpreter::Emit(const Compare<T>& op) {
  T left = swl::get<scalar_or_pointer_t<T>>(vars.Get(op.left_idx)).value;
  T right;
  if (op.right.IsVar()) {
    right = swl::get<scalar_or_pointer_t<T>>(vars.Get(op.right.GetVar())).value;
  }
  else {
    right = op.right.GetScalar();
  }
  calyx_op_type(op)::result_t result;

  if constexpr(std::is_same_v<T, calyx::Pointer>) {
    auto lptr = ReadPointer(left.value);
    auto rptr = ReadPointer(right.value);

    if (swl::holds_alternative<u8*>(lptr) || swl::holds_alternative<u8*>(rptr)) {
      throw cotyl::UnimplementedException("Comparing u8* pointers");
    }

    if (swl::holds_alternative<LabelOffset>(lptr) && swl::holds_alternative<LabelOffset>(rptr)) {
      LabelOffset llab = swl::get<LabelOffset>(lptr);
      LabelOffset rlab = swl::get<LabelOffset>(rptr);
      if (llab.label == rlab.label) {
        // same label, values are the same if and only if the offset is the same
        result = llab.offset == rlab.offset;
      }
      else if (llab.offset == rlab.offset) {
        // same offset with different labels is always false
        result = false;
      }
      else {
        throw InterpreterError("Cannot compare pointers to different symbols with varying offsets");
      }
    }
    else {
      i64 lval, rval;
      if (swl::holds_alternative<i64>(lptr)) {
        lval = swl::get<i64>(lptr);
        if (swl::holds_alternative<i64>(rptr)) {
          rval = swl::get<i64>(rptr);
        }
        else {
          if (lval != 0) throw InterpreterError("Comparing label to non-zero pointer");
          // 0 is ALWAYS less and not equal to label, regardless of the label
          rval = 1;
        }
      }
      else {
        // must have rptr = i64, lptr = LabelOffset
        rval = swl::get<i64>(rptr);
        if (rval != 0) throw InterpreterError("Comparing label to non-zero pointer");;
        // 0 is ALWAYS less and not equal to label, regardless of the label
        lval = 1;
      }

      switch (op.op) {
        case CmpType::Eq: result = lval == rval; break;
        case CmpType::Ne: result = lval != rval; break;
        case CmpType::Lt: result = lval <  rval; break;
        case CmpType::Le: result = lval <= rval; break;
        case CmpType::Gt: result = lval >  rval; break;
        case CmpType::Ge: result = lval >= rval; break;
      }
    }
  }
  else {
    switch (op.op) {
      case CmpType::Eq: result = left == right; break;
      case CmpType::Ne: result = left != right; break;
      case CmpType::Lt: result = left <  right; break;
      case CmpType::Le: result = left <= right; break;
      case CmpType::Gt: result = left >  right; break;
      case CmpType::Ge: result = left >= right; break;
    }
  }
  vars.Set(op.idx, Scalar<i32>{result});
}

void Interpreter::Emit(const UnconditionalBranch& op) {
  pos.pos.first  = op.dest;
  pos.pos.second = 0;
}

template<typename T>
void Interpreter::Emit(const BranchCompare<T>& op) {
  T left = swl::get<scalar_or_pointer_t<T>>(vars.Get(op.left_idx)).value;
  T right;
  if (op.right.IsVar()) {
    right = swl::get<scalar_or_pointer_t<T>>(vars.Get(op.right.GetVar())).value;
  }
  else {
    right = op.right.GetScalar();
  }
  bool branch;

  if constexpr(std::is_same_v<T, Pointer>) {
    auto lptr = ReadPointer(left.value);
    auto rptr = ReadPointer(right.value);

    if (swl::holds_alternative<u8*>(lptr) || swl::holds_alternative<u8*>(rptr)) {
      throw cotyl::UnimplementedException("Comparing u8* pointers");
    }

    if (swl::holds_alternative<i64>(lptr) && swl::holds_alternative<i64>(rptr)) {
      auto lval = swl::get<i64>(lptr);
      auto rval = swl::get<i64>(rptr);
      switch (op.op) {
        case CmpType::Eq: branch = lval == rval; break;
        case CmpType::Ne: branch = lval != rval; break;
        case CmpType::Lt: branch = lval <  rval; break;
        case CmpType::Le: branch = lval <= rval; break;
        case CmpType::Gt: branch = lval >  rval; break;
        case CmpType::Ge: branch = lval >= rval; break;
      }
    }
    else {
      throw cotyl::UnimplementedException("symbol compare");
    }
  }
  else {
    switch (op.op) {
      case calyx::CmpType::Eq: branch = left == right; break;
      case calyx::CmpType::Ne: branch = left != right; break;
      case calyx::CmpType::Gt: branch = left >  right; break;
      case calyx::CmpType::Ge: branch = left >= right; break;
      case calyx::CmpType::Lt: branch = left <  right; break;
      case calyx::CmpType::Le: branch = left <= right; break;
    }
  }

  pos.pos.first = branch ? op.tdest : op.fdest;
  pos.pos.second = 0;
}

void Interpreter::Emit(const Select& op) {
  cotyl::Assert(vars.HasTop(op.idx));
  auto val = swl::get<Scalar<calyx_op_type(op)::src_t>>(vars.Get(op.idx)).value;
  cotyl::Assert(op._default || op.table->contains(val), "Jump table does not contain value");
  if (op.table->contains(val)) {
    pos.pos.first  = op.table->at(val);
  }
  else {
    pos.pos.first = op._default;
  }
  pos.pos.second = 0;
}

template<typename T>
void Interpreter::Emit(const AddToPointer<T>& op) {
  calyx::Pointer left;
  if (op.ptr.IsVar()) {
    left = swl::get<calyx::Pointer>(vars.Get(op.ptr.GetVar()));
  }
  else {
    left = op.ptr.GetScalar();
  }
  const auto lptr = ReadPointer(left.value);
  calyx_op_type(op)::offset_t right;
  if (op.right.IsVar()) {
    right = swl::get<Scalar<calyx_op_type(op)::offset_t>>(vars.Get(op.right.GetVar())).value;
  }
  else {
    right = op.right.GetScalar();
  }
  calyx::Pointer result;

  swl::visit(
    swl::overloaded{
      [&](const i64& pval) {
        result = MakePointer(pval + (i64)op.stride * (i64)right);
      },
      [&](const LabelOffset& pval) {
        result = MakePointer(LabelOffset{cotyl::CString(pval.label), pval.offset + (i64) op.stride * (i64) right});
      },
      [&](u8* const& pval) {
        result = MakePointer(pval + (i64)op.stride * (i64)right);
      },
      swl::exhaustive
    },
    lptr
  );
  vars.Set(op.idx, result);
}

}