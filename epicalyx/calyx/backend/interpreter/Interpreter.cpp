#include "Interpreter.h"

#include "Assert.h"

#include <stdexcept>
#include <iostream>
#include <algorithm>
#include <limits>


namespace epi::calyx {


void Interpreter::EmitProgram(std::vector<calyx::pDirective>& program) {
  for (auto& directive : program) {
    directive->Emit(*this);
  }
}

void Interpreter::Emit(AllocateCVar& op) {
  cotyl::Assert(op.c_idx == c_vars.size(), op.ToString());
  c_vars.push_back(stack.size());
  stack.resize(stack.size() + op.size);
}

void Interpreter::Emit(DeallocateCVar& op) {
  u64 value = 0;
  memcpy(&value, &stack[c_vars[op.c_idx]], op.size);
//  std::cout << 'c' << op.c_idx << " = " << std::hex << value << " on dealloc" << std::endl;
//  stack.resize(stack.size() - op.size);
}

void Interpreter::Emit(LoadCVarAddr& op) {
  throw std::runtime_error("Unimplemented interpreter: load cvar");
}

template<typename To, typename From>
void Interpreter::EmitCast(Cast<To, From>& op) {
  cotyl::Assert(op.idx == vars.size(), op.ToString());
  if constexpr(std::is_same_v<To, Pointer>) {

  }
  else if constexpr(std::is_same_v<From, Pointer>) {

  }
  else {
    To value;
    From from = std::get<From>(vars[op.right_idx]);
    if constexpr(std::is_floating_point_v<From>) {
      if constexpr(std::is_floating_point_v<To>) {
        value = (To)from;
      }
      else {
        // out of bounds is UB anyway...
        value = std::clamp<From>(from, std::numeric_limits<To>::min(), std::numeric_limits<To>::max());
      }
    }
    else {
      value = from;
    }
    vars.push_back((calyx::calyx_upcast_t<To>)value);
  }
}

template<typename T>
void Interpreter::EmitLoadCVar(LoadCVar<T>& op) {
  cotyl::Assert(op.idx == vars.size(), op.ToString());
  if constexpr(std::is_same_v<T, Pointer>) {

  }
  else if constexpr(std::is_same_v<T, Struct>) {

  }
  else {
    T value;
    memcpy(&value, &stack[c_vars[op.c_idx]], sizeof(T));
    vars.push_back((calyx_upcast_t<T>)value);
  }
}

template<typename T>
void Interpreter::EmitStoreCVar(StoreCVar<T>& op) {
  cotyl::Assert(op.idx == vars.size(), op.ToString());
  if constexpr(std::is_same_v<T, Pointer>) {

  }
  else if constexpr(std::is_same_v<T, Struct>) {

  }
  else {
    T value = (T)std::get<calyx_upcast_t<T>>(vars[op.src]);
    memcpy(&stack[c_vars[op.c_idx]], &value, sizeof(T));
    vars.push_back((calyx_upcast_t<T>)value);
  }
}

template<typename T>
void Interpreter::EmitReturn(Return<T>& op) {
  if constexpr(std::is_same_v<T, Pointer>) {

  }
  else if constexpr(std::is_same_v<T, Struct>) {

  }
  else {
    std::cout << "return " << std::get<T>(vars[op.idx]) << std::endl;
  }
}

template<typename T>
void Interpreter::EmitImm(Imm<T>& op) {
  cotyl::Assert(op.idx == vars.size(), op.ToString());
  vars.push_back(op.value);
}

template<typename T>
void Interpreter::EmitUnop(Unop<T>& op) {
  cotyl::Assert(op.idx == vars.size(), op.ToString());
  T right = std::get<T>(vars[op.right_idx]);
  switch (op.op) {
    case UnopType::Neg:
      vars.push_back((T)-right); break;
    case UnopType::BinNot:
      if constexpr(std::is_integral_v<T>) {
        vars.push_back((T)~right); break;
      }
      else {
        throw std::runtime_error("floating point operand for binary not");
      }
  }
}

template<typename T>
void Interpreter::EmitBinop(Binop<T>& op) {
  cotyl::Assert(op.idx == vars.size(), op.ToString());
  T left = std::get<T>(vars[op.left_idx]);
  T right = std::get<T>(vars[op.right_idx]);
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
  vars.push_back(result);
}

template<typename T>
void Interpreter::EmitBinopImm(BinopImm<T>& op) {
  cotyl::Assert(op.idx == vars.size(), op.ToString());
  T left = std::get<T>(vars[op.left_idx]);
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
  vars.push_back(result);
}

template<typename T>
void Interpreter::EmitShift(Shift<T>& op) {
  cotyl::Assert(op.idx == vars.size(), op.ToString());
  T left = std::get<T>(vars[op.left_idx]);
  u32 right = std::get<u32>(vars[op.right_idx]);
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
  vars.push_back(left);
}

template<typename T>
void Interpreter::EmitAddToPointer(AddToPointer<T>& op) {

}


void Interpreter::Emit(Binop<i32>& op) { EmitBinop(op); }
void Interpreter::Emit(Binop<u32>& op) { EmitBinop(op); }
void Interpreter::Emit(Binop<i64>& op) { EmitBinop(op); }
void Interpreter::Emit(Binop<u64>& op) { EmitBinop(op); }
void Interpreter::Emit(Binop<float>& op) { EmitBinop(op); }
void Interpreter::Emit(Binop<double>& op) { EmitBinop(op); }
void Interpreter::Emit(BinopImm<i32>& op) { EmitBinopImm(op); }
void Interpreter::Emit(BinopImm<u32>& op) { EmitBinopImm(op); }
void Interpreter::Emit(BinopImm<i64>& op) { EmitBinopImm(op); }
void Interpreter::Emit(BinopImm<u64>& op) { EmitBinopImm(op); }
void Interpreter::Emit(BinopImm<float>& op) { EmitBinopImm(op); }
void Interpreter::Emit(BinopImm<double>& op) { EmitBinopImm(op); }
void Interpreter::Emit(Shift<i32>& op) { EmitShift(op); }
void Interpreter::Emit(Shift<u32>& op) { EmitShift(op); }
void Interpreter::Emit(Shift<i64>& op) { EmitShift(op); }
void Interpreter::Emit(Shift<u64>& op) { EmitShift(op); }
void Interpreter::Emit(AddToPointer<i32>& op) { EmitAddToPointer(op); }
void Interpreter::Emit(AddToPointer<u32>& op) { EmitAddToPointer(op); }
void Interpreter::Emit(AddToPointer<i64>& op) { EmitAddToPointer(op); }
void Interpreter::Emit(AddToPointer<u64>& op) { EmitAddToPointer(op); }
void Interpreter::Emit(Unop<i32>& op) { EmitUnop(op); }
void Interpreter::Emit(Unop<u32>& op) { EmitUnop(op); }
void Interpreter::Emit(Unop<i64>& op) { EmitUnop(op); }
void Interpreter::Emit(Unop<u64>& op) { EmitUnop(op); }
void Interpreter::Emit(Unop<float>& op) { EmitUnop(op); }
void Interpreter::Emit(Unop<double>& op) { EmitUnop(op); }
void Interpreter::Emit(Imm<i32>& op) { EmitImm(op); }
void Interpreter::Emit(Imm<u32>& op) { EmitImm(op); }
void Interpreter::Emit(Imm<i64>& op) { EmitImm(op); }
void Interpreter::Emit(Imm<u64>& op) { EmitImm(op); }
void Interpreter::Emit(Imm<float>& op) { EmitImm(op); }
void Interpreter::Emit(Imm<double>& op) { EmitImm(op); }
void Interpreter::Emit(LoadCVar<i8>& op) { EmitLoadCVar(op); }
void Interpreter::Emit(LoadCVar<u8>& op) { EmitLoadCVar(op); }
void Interpreter::Emit(LoadCVar<i16>& op) { EmitLoadCVar(op); }
void Interpreter::Emit(LoadCVar<u16>& op) { EmitLoadCVar(op); }
void Interpreter::Emit(LoadCVar<i32>& op) { EmitLoadCVar(op); }
void Interpreter::Emit(LoadCVar<u32>& op) { EmitLoadCVar(op); }
void Interpreter::Emit(LoadCVar<i64>& op) { EmitLoadCVar(op); }
void Interpreter::Emit(LoadCVar<u64>& op) { EmitLoadCVar(op); }
void Interpreter::Emit(LoadCVar<float>& op) { EmitLoadCVar(op); }
void Interpreter::Emit(LoadCVar<double>& op) { EmitLoadCVar(op); }
void Interpreter::Emit(LoadCVar<Struct>& op) { EmitLoadCVar(op); }
void Interpreter::Emit(LoadCVar<Pointer>& op) { EmitLoadCVar(op); }
void Interpreter::Emit(StoreCVar<i8>& op) { EmitStoreCVar(op); }
void Interpreter::Emit(StoreCVar<u8>& op) { EmitStoreCVar(op); }
void Interpreter::Emit(StoreCVar<i16>& op) { EmitStoreCVar(op); }
void Interpreter::Emit(StoreCVar<u16>& op) { EmitStoreCVar(op); }
void Interpreter::Emit(StoreCVar<i32>& op) { EmitStoreCVar(op); }
void Interpreter::Emit(StoreCVar<u32>& op) { EmitStoreCVar(op); }
void Interpreter::Emit(StoreCVar<i64>& op) { EmitStoreCVar(op); }
void Interpreter::Emit(StoreCVar<u64>& op) { EmitStoreCVar(op); }
void Interpreter::Emit(StoreCVar<float>& op) { EmitStoreCVar(op); }
void Interpreter::Emit(StoreCVar<double>& op) { EmitStoreCVar(op); }
void Interpreter::Emit(StoreCVar<Struct>& op) { EmitStoreCVar(op); }
void Interpreter::Emit(StoreCVar<Pointer>& op) { EmitStoreCVar(op); }
void Interpreter::Emit(Return<i32>& op) { EmitReturn(op); }
void Interpreter::Emit(Return<u32>& op) { EmitReturn(op); }
void Interpreter::Emit(Return<i64>& op) { EmitReturn(op); }
void Interpreter::Emit(Return<u64>& op) { EmitReturn(op); }
void Interpreter::Emit(Return<float>& op) { EmitReturn(op); }
void Interpreter::Emit(Return<double>& op) { EmitReturn(op); }
void Interpreter::Emit(Return<Struct>& op) { EmitReturn(op); }
void Interpreter::Emit(Return<Pointer>& op) { EmitReturn(op); }
void Interpreter::Emit(Cast<i8, i32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i8, u32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i8, i64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i8, u64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i8, float>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i8, double>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i8, Pointer>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i16, i32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i16, u32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i16, i64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i16, u64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i16, float>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i16, double>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i16, Pointer>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i32, i32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i32, u32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i32, i64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i32, u64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i32, float>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i32, double>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i32, Pointer>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i64, i32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i64, u32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i64, i64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i64, u64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i64, float>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i64, double>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<i64, Pointer>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u8, i32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u8, u32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u8, i64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u8, u64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u8, float>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u8, double>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u8, Pointer>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u16, i32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u16, u32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u16, i64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u16, u64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u16, float>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u16, double>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u16, Pointer>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u32, i32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u32, u32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u32, i64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u32, u64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u32, float>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u32, double>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u32, Pointer>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u64, i32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u64, u32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u64, i64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u64, u64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u64, float>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u64, double>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<u64, Pointer>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<float, i32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<float, u32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<float, i64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<float, u64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<float, float>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<float, double>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<float, Pointer>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<double, i32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<double, u32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<double, i64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<double, u64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<double, float>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<double, double>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<double, Pointer>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<Pointer, i32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<Pointer, u32>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<Pointer, i64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<Pointer, u64>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<Pointer, float>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<Pointer, double>& op) { EmitCast(op); }
void Interpreter::Emit(Cast<Pointer, Pointer>& op) { EmitCast(op); }


}