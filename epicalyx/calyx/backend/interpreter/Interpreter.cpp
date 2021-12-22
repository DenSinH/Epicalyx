#include "Interpreter.h"

#include "cycle/Cycle.h"
#include "Assert.h"
#include "Format.h"
#include "Cast.h"

#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <limits>


namespace epi::calyx {

void Interpreter::VisualizeProgram(const Program& program) {
  auto graph = std::make_unique<epi::cycle::Graph>();

  for (const auto& [i, block] : program.blocks) {
    for (const auto& directive : block) {
      switch (directive->cls) {
        case Directive::Class::Expression:
        case Directive::Class::Stack:
        case Directive::Class::Store:
        case Directive::Class::Return:
        case Directive::Class::Call:  // todo
          graph->n(i, directive->ToString());
          break;
        case Directive::Class::Branch:
        case Directive::Class::UnconditionalBranch:
          graph->n(i, directive->ToString())->n(cotyl::unique_ptr_cast<Branch>(directive)->dest);
          break;
        case Directive::Class::Select: {
          auto node = graph->n(i, directive->ToString());
          auto* select = cotyl::unique_ptr_cast<Select>(directive);
          for (auto [val, block] : select->table) {
            node->n(block, std::to_string(val));
          }
          if (select->_default) {
            node->n(select->_default, "default");
          }
          break;
        }
      }
    }
  }

  for (const auto& [symbol, block] : program.functions) {
    graph->n(block).title(symbol + "(*)");
  }

  for (const auto& [symbol, global_init] : program.global_init) {
    if (std::holds_alternative<calyx::block_label_t>(global_init)) {
      auto block = std::get<calyx::block_label_t>(global_init);
      graph->n(block)->title("init " + symbol);
    }
  }

  graph->Visualize();
  graph->Join();
}

void Interpreter::EmitProgram(Program& program) {
  for (const auto& [symbol, size] : program.globals) {
    auto index = global_data.size();
    globals.emplace(symbol, index);
    global_data.push_back(std::vector<u8>(size));
  }

  for (const auto& [symbol, global_init] : program.global_init) {
    if (std::holds_alternative<std::vector<u8>>(global_init)) {
      const auto& data = std::get<std::vector<u8>>(global_init);
      std::memcpy(global_data[globals.at(symbol)].data(), data.data(), global_data[globals.at(symbol)].size());
    }
    else if (std::holds_alternative<calyx::label_offset_t>(global_init)) {
      throw std::runtime_error("Unimplemented: global label offset init");
    }
    else if (std::holds_alternative<calyx::block_label_t>(global_init)) {
      pos.first = std::get<calyx::block_label_t>(global_init);
      pos.second = 0;
      call_stack.emplace(std::make_pair(0, 0), -1, arg_list_t{}, arg_list_t{});
      vars.NewLayer();
      locals.NewLayer();

      while (pos.first) {
        auto& directive = program.blocks[pos.first][pos.second];
        pos.second++;
        directive->Emit(*this);
      }

      auto _symbol = symbol;
      std::visit([&](auto& var) {
        std::memcpy(global_data[globals.at(_symbol)].data(), &var, global_data[globals.at(_symbol)].size());
      }, vars.Get(-1));

      vars.Reset();
      locals.Reset();
    }
    else {
      throw std::runtime_error("Bad global initializer");
    }
  }

  pos.first = program.functions.at("main");
  pos.second = 0;
  returned = {};
  while (!returned) {
    auto& directive = program.blocks[pos.first][pos.second];
    pos.second++;
    directive->Emit(*this);
  }

  std::visit([&](auto& var) {
    if constexpr(std::is_same_v<decltype(var), Pointer&>) {
      std::cout << "return pointer" << std::endl;
    }
    else if constexpr(std::is_same_v<decltype(var), Struct&>) {
      std::cout << "return struct" << std::endl;
    }
    else {
      std::cout << "return " << var << std::endl;
    }
  }, returned.value());
}

void Interpreter::Emit(AllocateLocal& op) {
//  cotyl::Assert(!c_vars.contains(op.loc_idx), op.ToString());
  locals.Set(op.loc_idx, std::make_pair(stack.size(), op.size));
  stack.resize(stack.size() + op.size);
}

void Interpreter::Emit(DeallocateLocal& op) {
  u64 value = 0;
  memcpy(&value, &stack[locals.Get(op.loc_idx).first], op.size);
//  std::cout << 'c' << op.loc_idx << " = " << std::hex << value << " on dealloc" << std::endl;
//  stack.resize(stack.size() - op.size);
}

template<typename To, typename From>
void Interpreter::EmitCast(Cast<To, From>& op) {
//  cotyl::Assert(!vars.contains(op.idx), op.ToString());
  if constexpr(std::is_same_v<To, Pointer>) {
    To value;
    From from = std::get<From>(vars.Get(op.right_idx));
    if constexpr(std::is_same_v<From, Pointer>) {
      vars.Set(op.idx, MakePointer(ReadPointer(from.value)));
    }
    else {
      vars.Set(op.idx, MakePointer((i64)from));
    }
  }
  else if constexpr(std::is_same_v<From, Pointer>) {
    // we know that To is not a pointer type
    To value;
    From from = std::get<From>(vars.Get(op.right_idx));

    // assume we don't get label offsets here
    value = std::get<i64>(ReadPointer(from.value));
    vars.Get(op.idx) = value;
  }
  else {
    To value;
    From from = std::get<From>(vars.Get(op.right_idx));
    if constexpr(std::is_floating_point_v<From>) {
      if constexpr(std::is_floating_point_v<To>) {
        value = (To)from;
      }
      else {
        // out of bounds is UB anyway...
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-const-int-float-conversion"
        value = std::clamp<From>(from, std::numeric_limits<To>::min(), std::numeric_limits<To>::max());
#pragma clang diagnostic pop
      }
    }
    else {
      value = from;
    }
    vars.Set(op.idx, (calyx::calyx_upcast_t<To>)value);
  }
}

template<typename T>
void Interpreter::EmitLoadLocal(LoadLocal<T>& op) {
//  cotyl::Assert(!vars.contains(op.idx), op.ToString());
  if constexpr(std::is_same_v<T, Struct>) {
    throw std::runtime_error("Unimplemented: load struct cvar");
  }
  else {
    // works the same for pointers
    T value;
    memcpy(&value, &stack[locals.Get(op.loc_idx).first], sizeof(T));
    vars.Set(op.idx, (calyx_upcast_t<T>)value);
  }
}

void Interpreter::Emit(LoadLocalAddr& op) {
  vars.Set(op.idx, MakePointer(locals.Get(op.loc_idx).first));
}

template<typename T>
void Interpreter::EmitStoreLocal(StoreLocal<T>& op) {
//  cotyl::Assert(!vars.contains(op.idx), op.ToString());
  if constexpr(std::is_same_v<T, Struct>) {
    throw std::runtime_error("Unimplemented: store struct local");
  }
  else {
    // works the same for pointers
    T value = (T)std::get<calyx_upcast_t<T>>(vars.Get(op.src));
    memcpy(&stack[locals.Get(op.loc_idx).first], &value, sizeof(T));
    vars.Set(op.idx, (calyx_upcast_t<T>)value);
  }
}

template<typename T>
void Interpreter::EmitLoadGlobal(LoadGlobal<T>& op) {
  if constexpr(std::is_same_v<T, Struct>) {
    throw std::runtime_error("Unimplemented: load struct global");
  }
  else {
    cotyl::Assert(global_data[globals.at(op.symbol)].size() == sizeof(T));
    T value;
    std::memcpy(&value, global_data[globals.at(op.symbol)].data(), sizeof(T));
    vars.Set(op.idx, (calyx_upcast_t<T>)value);
  };
}

void Interpreter::Emit(LoadGlobalAddr& op) {
  if (program.functions.contains(op.symbol)) {
    vars.Set(op.idx, MakePointer(calyx::label_offset_t{op.symbol, 0}));
  }
  else {
    vars.Set(op.idx, MakePointer(-globals.at(op.symbol)));
  }
}

template<typename T>
void Interpreter::EmitStoreGlobal(StoreGlobal<T>& op) {
  if constexpr(std::is_same_v<T, Struct>) {
    throw std::runtime_error("Unimplemented: load struct global");
  }
  else {
    cotyl::Assert(global_data[globals.at(op.symbol)].size() == sizeof(T));
    T value = (T)std::get<calyx_upcast_t<T>>(vars.Get(op.src));
    std::memcpy(global_data[globals.at(op.symbol)].data(), &value, sizeof(T));
    vars.Set(op.idx, (calyx_upcast_t<T>)value);
  };
}

template<typename T>
void Interpreter::EmitLoadFromPointer(LoadFromPointer<T>& op) {
  auto pointer = ReadPointer(std::get<Pointer>(vars.Get(op.ptr_idx)).value);
  if constexpr(std::is_same_v<T, Struct>) {
    throw std::runtime_error("Unimplemented: load struct from pointer");
  }
  else {
    T value;
    if (std::holds_alternative<i64>(pointer)) {
      const auto pval = std::get<i64>(pointer);
      if (pval >= 0) {
        memcpy(&value, &stack[pval] + op.offset, sizeof(T));
      }
      else {
        cotyl::Assert(global_data[-pval].size() >= sizeof(T));
        memcpy(&value, global_data[-pval].data() + op.offset, sizeof(T));
      }
    }
    else {
      const auto pval = std::get<calyx::label_offset_t>(pointer);
      cotyl::Assert(global_data[globals[pval.label]].size() - op.offset - pval.offset >= sizeof(T));
      memcpy(&value, global_data[globals[pval.label]].data() + op.offset + pval.offset, sizeof(T));
    }
    vars.Set(op.idx, (calyx_upcast_t<T>)value);
  }
}

template<typename T>
void Interpreter::EmitStoreToPointer(StoreToPointer<T>& op) {
  auto pointer = ReadPointer(std::get<Pointer>(vars.Get(op.ptr_idx)).value);
  if constexpr(std::is_same_v<T, Struct>) {
    throw std::runtime_error("Unimplemented: store struct to pointer");
  }
  else {
    T value = std::get<calyx_upcast_t<T>>(vars.Get(op.src));

    if (std::holds_alternative<i64>(pointer)) {
      const auto pval = std::get<i64>(pointer);
      if (pval >= 0) {
        memcpy(&stack[pval + op.offset], &value, sizeof(T));
      }
      else {
        cotyl::Assert(global_data[-pval].size() >= sizeof(T));
        memcpy(global_data[-pval].data() + op.offset, &value, sizeof(T));
      }
    }
    else {
      const auto pval = std::get<calyx::label_offset_t>(pointer);
      cotyl::Assert(global_data[globals[pval.label]].size() - op.offset - pval.offset >= sizeof(T));
      memcpy(global_data[globals[pval.label]].data() + op.offset + pval.offset, &value, sizeof(T));
    }
  }
}

template<typename T>
void Interpreter::EmitCall(Call<T>& op) {
  call_stack.emplace(pos, op.idx, op.args, op.var_args);
  auto pointer = ReadPointer(std::get<Pointer>(vars.Get(op.fn_idx)).value);
  if (std::holds_alternative<i64>(pointer)) {
    pos.first = std::get<i64>(pointer);
  }
  else {
    auto pval = std::get<calyx::label_offset_t>(pointer);
    pos.first = program.functions.at(pval.label);
    cotyl::Assert(pval.offset == 0, "Cannot jump to offset label in call");
  }

  pos.second = 0;
  locals.NewLayer();
  vars.NewLayer();
}

template<typename T>
void Interpreter::EmitCallLabel(CallLabel<T>& op) {
  call_stack.emplace(pos, op.idx, op.args, op.var_args);
  pos.first = program.functions.at(op.label);
  pos.second = 0;
  locals.NewLayer();
  vars.NewLayer();
}

void Interpreter::Emit(ArgMakeLocal& op) {
  auto [_, __, args, ___] = call_stack.top();

  const auto stack_loc = stack.size();
  switch (op.arg.type) {
    case Argument::Type::I8: {
      i32 value = std::get<i32>(vars.Get(args[op.arg.arg_idx].first));
      locals.Set(op.loc_idx, std::make_pair(stack_loc, sizeof(value)));
      stack.resize(stack.size() + sizeof(value));
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Argument::Type::U8: {
      u32 value = std::get<u32>(vars.Get(args[op.arg.arg_idx].first));
      locals.Set(op.loc_idx, std::make_pair(stack_loc, sizeof(value)));
      stack.resize(stack.size() + sizeof(value));
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Argument::Type::I16: {
      i32 value = std::get<i32>(vars.Get(args[op.arg.arg_idx].first));
      locals.Set(op.loc_idx, std::make_pair(stack_loc, sizeof(value)));
      stack.resize(stack.size() + sizeof(value));
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Argument::Type::U16: {
      u32 value = std::get<u32>(vars.Get(args[op.arg.arg_idx].first));
      locals.Set(op.loc_idx, std::make_pair(stack_loc, sizeof(value)));
      stack.resize(stack.size() + sizeof(value));
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Argument::Type::I32: {
      i32 value = std::get<i32>(vars.Get(args[op.arg.arg_idx].first));
      locals.Set(op.loc_idx, std::make_pair(stack_loc, sizeof(value)));
      stack.resize(stack.size() + sizeof(value));
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Argument::Type::U32: {
      u32 value = std::get<u32>(vars.Get(args[op.arg.arg_idx].first));
      locals.Set(op.loc_idx, std::make_pair(stack_loc, sizeof(value)));
      stack.resize(stack.size() + sizeof(value));
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Argument::Type::I64: {
      i64 value = std::get<i64>(vars.Get(args[op.arg.arg_idx].first));
      locals.Set(op.loc_idx, std::make_pair(stack_loc, sizeof(value)));
      stack.resize(stack.size() + sizeof(value));
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Argument::Type::U64: {
      u64 value = std::get<u64>(vars.Get(args[op.arg.arg_idx].first));
      locals.Set(op.loc_idx, std::make_pair(stack_loc, sizeof(value)));
      stack.resize(stack.size() + sizeof(value));
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Argument::Type::Float: {
      float value = std::get<float>(vars.Get(args[op.arg.arg_idx].first));
      locals.Set(op.loc_idx, std::make_pair(stack_loc, sizeof(value)));
      stack.resize(stack.size() + sizeof(value));
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Argument::Type::Double: {
      double value = std::get<double>(vars.Get(args[op.arg.arg_idx].first));
      locals.Set(op.loc_idx, std::make_pair(stack_loc, sizeof(value)));
      stack.resize(stack.size() + sizeof(value));
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Argument::Type::Pointer: {
      Pointer value = std::get<Pointer>(vars.Get(args[op.arg.arg_idx].first));
      locals.Set(op.loc_idx, std::make_pair(stack_loc, sizeof(value)));
      stack.resize(stack.size() + sizeof(value));
      std::memcpy(&stack[stack_loc], &value, sizeof(value));
      break;
    }
    case Argument::Type::Struct: {
      throw std::runtime_error("Unimplemented: struct argument");
    }
  }
}

template<typename T>
void Interpreter::EmitReturn(Return<T>& op) {
  auto top_vars = std::move(vars.Top());
  auto top_locals = std::move(locals.Top());
  locals.PopLayer();
  vars.PopLayer();

  if (call_stack.empty()) {
    returned = top_vars.at(op.idx);
  }
  else {
    auto [_pos, return_to, _, __] = call_stack.top();
    call_stack.pop();
    pos = _pos;
    if constexpr(!std::is_same_v<T, void>) {
      vars.Set(return_to, top_vars.at(op.idx));
    }
  }

  for (const auto& [idx, local] : top_locals) {
    stack.resize(stack.size() - local.second);
  }
}

template<typename T>
void Interpreter::EmitImm(Imm<T>& op) {
//  cotyl::Assert(!vars.contains(op.idx), op.ToString());
  vars.Set(op.idx, op.value);
}

template<typename T>
void Interpreter::EmitUnop(Unop<T>& op) {
//  cotyl::Assert(!vars.contains(op.idx), op.ToString());
  T right = std::get<T>(vars.Get(op.right_idx));
  switch (op.op) {
    case UnopType::Neg:
      vars.Set(op.idx, (T)-right); break;
    case UnopType::BinNot:
      if constexpr(std::is_integral_v<T>) {
        vars.Set(op.idx, (T)~right); break;
      }
      else {
        throw std::runtime_error("floating point operand for binary not");
      }
  }
}

template<typename T>
void Interpreter::EmitBinop(Binop<T>& op) {
//  cotyl::Assert(!vars.contains(op.idx), op.ToString());
  T left = std::get<T>(vars.Get(op.left_idx));
  T right = std::get<T>(vars.Get(op.right_idx));
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
        throw std::runtime_error("Float operands for mod expression");
      }
    }
    case BinopType::BinAnd:{
      if constexpr(std::is_integral_v<T>) {
        result = left & right;
        break;
      }
      else {
        throw std::runtime_error("Float operands for bin and expression");
      }
    }
    case BinopType::BinOr: {
      if constexpr(std::is_integral_v<T>) {
        result = left | right;
        break;
      }
      else {
        throw std::runtime_error("Float operands for bin or expression");
      }
    }
    case BinopType::BinXor:{
      if constexpr(std::is_integral_v<T>) {
        result = left ^ right;
        break;
      }
      else {
        throw std::runtime_error("Float operands for bin xor expression");
      }
    }
  }
  vars.Set(op.idx, result);
}

template<typename T>
void Interpreter::EmitBinopImm(BinopImm<T>& op) {
//  cotyl::Assert(!vars.contains(op.idx), op.ToString());
  T left = std::get<T>(vars.Get(op.left_idx));
  T result;
  switch (op.op) {
    case BinopType::Add: result = left + op.right; break;
    case BinopType::Sub: result = left - op.right; break;
    case BinopType::Mul: result = left * op.right; break;
    case BinopType::Div: result = left / op.right; break;
    case BinopType::Mod: {
      if constexpr(std::is_integral_v<T>) {
        result = left % op.right;
        break;
      }
      else {
        throw std::runtime_error("Float operands for mod expression");
      }
    }
    case BinopType::BinAnd:{
      if constexpr(std::is_integral_v<T>) {
        result = left & op.right;
        break;
      }
      else {
        throw std::runtime_error("Float operands for bin and expression");
      }
    }
    case BinopType::BinOr: {
      if constexpr(std::is_integral_v<T>) {
        result = left | op.right;
        break;
      }
      else {
        throw std::runtime_error("Float operands for bin or expression");
      }
    }
    case BinopType::BinXor:{
      if constexpr(std::is_integral_v<T>) {
        result = left ^ op.right;
        break;
      }
      else {
        throw std::runtime_error("Float operands for bin xor expression");
      }
    }
  }
  vars.Set(op.idx, result);
}

template<typename T>
void Interpreter::EmitShift(Shift<T>& op) {
//  cotyl::Assert(!vars.contains(op.idx), op.ToString());
  T left = std::get<T>(vars.Get(op.left_idx));
  u32 right = std::get<u32>(vars.Get(op.right_idx));
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
  vars.Set(op.idx, left);
}

template<typename T>
void Interpreter::EmitShiftImm(ShiftImm<T>& op) {
//  cotyl::Assert(!vars.contains(op.idx), op.ToString());
  T left = std::get<T>(vars.Get(op.left_idx));
  switch (op.op) {
    case calyx::ShiftType::Left: {
      left <<= op.right;
      break;
    }
    case calyx::ShiftType::Right: {
      left >>= op.right;
      break;
    }
  }
  vars.Set(op.idx, left);
}

template<typename T>
void Interpreter::EmitCompare(Compare<T>& op) {
  T left = std::get<T>(vars.Get(op.left_idx));
  T right = std::get<T>(vars.Get(op.right_idx));
  i32 result;

  if constexpr(std::is_same_v<T, calyx::Pointer>) {
    auto lptr = ReadPointer(left.value);
    auto rptr = ReadPointer(right.value);
    if (std::holds_alternative<i64>(lptr) && std::holds_alternative<i64>(rptr)) {
      auto lval = std::get<i64>(lptr);
      auto rval = std::get<i64>(rptr);
      switch (op.op) {
        case CmpType::Eq: result = lval == rval; break;
        case CmpType::Ne: result = lval != rval; break;
        case CmpType::Lt: result = lval <  rval; break;
        case CmpType::Le: result = lval <= rval; break;
        case CmpType::Gt: result = lval >  rval; break;
        case CmpType::Ge: result = lval >= rval; break;
      }
    }
    else {
      throw std::runtime_error("unimplemented: symbol compare");
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
  vars.Set(op.idx, result);
}

template<typename T>
void Interpreter::EmitCompareImm(CompareImm<T>& op) {
  T left = std::get<T>(vars.Get(op.left_idx));
  i32 result;

  if constexpr(std::is_same_v<T, calyx::Pointer>) {
    auto lptr = ReadPointer(left.value);
    auto rptr = ReadPointer(op.right.value);
    if (std::holds_alternative<i64>(lptr) && std::holds_alternative<i64>(rptr)) {
      auto lval = std::get<i64>(lptr);
      auto rval = std::get<i64>(rptr);
      switch (op.op) {
        case CmpType::Eq: result = lval == rval; break;
        case CmpType::Ne: result = lval != rval; break;
        case CmpType::Lt: result = lval <  rval; break;
        case CmpType::Le: result = lval <= rval; break;
        case CmpType::Gt: result = lval >  rval; break;
        case CmpType::Ge: result = lval >= rval; break;
      }
    }
    else {
      throw std::runtime_error("unimplemented: symbol compare");
    }
  }
  else {
    switch (op.op) {
      case CmpType::Eq: result = left == op.right; break;
      case CmpType::Ne: result = left != op.right; break;
      case CmpType::Lt: result = left <  op.right; break;
      case CmpType::Le: result = left <= op.right; break;
      case CmpType::Gt: result = left >  op.right; break;
      case CmpType::Ge: result = left >= op.right; break;
    }
  }
  vars.Set(op.idx, result);
}

void Interpreter::Emit(UnconditionalBranch& op) {
  pos.first  = op.dest;
  pos.second = 0;
}

template<typename T>
void Interpreter::EmitBranchCompare(BranchCompare<T>& op) {
  T left = std::get<T>(vars.Get(op.left_idx));
  T right = std::get<T>(vars.Get(op.right_idx));
  bool branch;

  if constexpr(std::is_same_v<T, Pointer>) {
    auto lptr = ReadPointer(left.value);
    auto rptr = ReadPointer(right.value);
    if (std::holds_alternative<i64>(lptr) && std::holds_alternative<i64>(rptr)) {
      auto lval = std::get<i64>(lptr);
      auto rval = std::get<i64>(rptr);
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
      throw std::runtime_error("unimplemented: symbol compare");
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

  if (branch) {
    pos.first = op.dest;
    pos.second = 0;
  }
}

template<typename T>
void Interpreter::EmitBranchCompareImm(BranchCompareImm<T>& op) {
  T left = std::get<T>(vars.Get(op.left_idx));
  T right = op.right;
  bool branch;

  if constexpr(std::is_same_v<T, Pointer>) {
    auto lptr = ReadPointer(left.value);
    auto rptr = ReadPointer(op.right.value);
    if (std::holds_alternative<i64>(lptr) && std::holds_alternative<i64>(rptr)) {
      auto lval = std::get<i64>(lptr);
      auto rval = std::get<i64>(rptr);
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
      throw std::runtime_error("unimplemented: symbol compare");
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

  if (branch) {
    pos.first = op.dest;
    pos.second = 0;
  }
}

void Interpreter::Emit(Select& op) {
  cotyl::Assert(vars.HasTop(op.idx));
  auto val = std::get<i64>(vars.Get(op.idx));
  cotyl::Assert(op._default || op.table.contains(val), "Jump table does not contain value");
  if (op.table.contains(val)) {
    pos.first  = op.table.at(val);
  }
  else {
    pos.first = op._default;
  }
  pos.second = 0;
}

template<typename T>
void Interpreter::EmitAddToPointer(AddToPointer<T>& op) {
  calyx::Pointer left = std::get<calyx::Pointer>(vars.Get(op.ptr_idx));
  const auto lptr = ReadPointer(left.value);
  T right = std::get<T>(vars.Get(op.right_idx));
  calyx::Pointer result;
  if (std::holds_alternative<i64>(lptr)) {
    result = MakePointer(std::get<i64>(lptr) + (i64)op.stride * (i64)right);
  }
  else {
    auto pval = std::get<calyx::label_offset_t>(lptr);
    result = MakePointer(calyx::label_offset_t{pval.label, pval.offset + (i64) op.stride * (i64) right});
  }
  vars.Set(op.idx, result);
}

void Interpreter::Emit(AddToPointerImm& op) {
  calyx::Pointer left = std::get<calyx::Pointer>(vars.Get(op.ptr_idx));
  const auto lptr = ReadPointer(left.value);
  i64 right = op.right;
  calyx::Pointer result;
  if (std::holds_alternative<i64>(lptr)) {
    result = MakePointer(std::get<i64>(lptr) + (i64)op.stride * (i64)right);
  }
  else {
    auto pval = std::get<calyx::label_offset_t>(lptr);
    result = MakePointer(calyx::label_offset_t{pval.label, pval.offset + (i64) op.stride * (i64) right});
  }
  vars.Set(op.idx, result);
}

#define BACKEND_NAME Interpreter
#include "Templates.inl"

}