#pragma once


#include "calyx/Calyx.h"

#include <vector>

namespace epi::calyx {

struct Backend {

  virtual void EmitProgram(std::vector<calyx::pDirective>& program) = 0;

  virtual void Emit(AllocateCVar& op) = 0;
  virtual void Emit(DeallocateCVar& op) = 0;
  virtual void Emit(LoadCVarAddr& op) = 0;

  virtual void Emit(Binop<i32>& op) = 0;
  virtual void Emit(Binop<u32>& op) = 0;
  virtual void Emit(Binop<i64>& op) = 0;
  virtual void Emit(Binop<u64>& op) = 0;
  virtual void Emit(Binop<float>& op) = 0;
  virtual void Emit(Binop<double>& op) = 0;
  virtual void Emit(AddToPointer<i32>& op) = 0;
  virtual void Emit(AddToPointer<u32>& op) = 0;
  virtual void Emit(AddToPointer<i64>& op) = 0;
  virtual void Emit(AddToPointer<u64>& op) = 0;
  virtual void Emit(Unop<i32>& op) = 0;
  virtual void Emit(Unop<u32>& op) = 0;
  virtual void Emit(Unop<i64>& op) = 0;
  virtual void Emit(Unop<u64>& op) = 0;
  virtual void Emit(Unop<float>& op) = 0;
  virtual void Emit(Unop<double>& op) = 0;
  virtual void Emit(Imm<i32>& op) = 0;
  virtual void Emit(Imm<u32>& op) = 0;
  virtual void Emit(Imm<i64>& op) = 0;
  virtual void Emit(Imm<u64>& op) = 0;
  virtual void Emit(Imm<float>& op) = 0;
  virtual void Emit(Imm<double>& op) = 0;
  virtual void Emit(LoadCVar<i8>& op) = 0;
  virtual void Emit(LoadCVar<u8>& op) = 0;
  virtual void Emit(LoadCVar<i16>& op) = 0;
  virtual void Emit(LoadCVar<u16>& op) = 0;
  virtual void Emit(LoadCVar<i32>& op) = 0;
  virtual void Emit(LoadCVar<u32>& op) = 0;
  virtual void Emit(LoadCVar<i64>& op) = 0;
  virtual void Emit(LoadCVar<u64>& op) = 0;
  virtual void Emit(LoadCVar<float>& op) = 0;
  virtual void Emit(LoadCVar<double>& op) = 0;
  virtual void Emit(LoadCVar<Struct>& op) = 0;
  virtual void Emit(LoadCVar<Pointer>& op) = 0;
  virtual void Emit(StoreCVar<i8>& op) = 0;
  virtual void Emit(StoreCVar<u8>& op) = 0;
  virtual void Emit(StoreCVar<i16>& op) = 0;
  virtual void Emit(StoreCVar<u16>& op) = 0;
  virtual void Emit(StoreCVar<i32>& op) = 0;
  virtual void Emit(StoreCVar<u32>& op) = 0;
  virtual void Emit(StoreCVar<i64>& op) = 0;
  virtual void Emit(StoreCVar<u64>& op) = 0;
  virtual void Emit(StoreCVar<float>& op) = 0;
  virtual void Emit(StoreCVar<double>& op) = 0;
  virtual void Emit(StoreCVar<Struct>& op) = 0;
  virtual void Emit(StoreCVar<Pointer>& op) = 0;
  virtual void Emit(Return<i32>& op) = 0;
  virtual void Emit(Return<u32>& op) = 0;
  virtual void Emit(Return<i64>& op) = 0;
  virtual void Emit(Return<u64>& op) = 0;
  virtual void Emit(Return<float>& op) = 0;
  virtual void Emit(Return<double>& op) = 0;
  virtual void Emit(Return<Struct>& op) = 0;
  virtual void Emit(Return<Pointer>& op) = 0;
  virtual void Emit(Cast<i8, i32>& op) = 0;
  virtual void Emit(Cast<i8, u32>& op) = 0;
  virtual void Emit(Cast<i8, i64>& op) = 0;
  virtual void Emit(Cast<i8, u64>& op) = 0;
  virtual void Emit(Cast<i8, float>& op) = 0;
  virtual void Emit(Cast<i8, double>& op) = 0;
  virtual void Emit(Cast<i8, Pointer>& op) = 0;
  virtual void Emit(Cast<i16, i32>& op) = 0;
  virtual void Emit(Cast<i16, u32>& op) = 0;
  virtual void Emit(Cast<i16, i64>& op) = 0;
  virtual void Emit(Cast<i16, u64>& op) = 0;
  virtual void Emit(Cast<i16, float>& op) = 0;
  virtual void Emit(Cast<i16, double>& op) = 0;
  virtual void Emit(Cast<i16, Pointer>& op) = 0;
  virtual void Emit(Cast<i32, i32>& op) = 0;
  virtual void Emit(Cast<i32, u32>& op) = 0;
  virtual void Emit(Cast<i32, i64>& op) = 0;
  virtual void Emit(Cast<i32, u64>& op) = 0;
  virtual void Emit(Cast<i32, float>& op) = 0;
  virtual void Emit(Cast<i32, double>& op) = 0;
  virtual void Emit(Cast<i32, Pointer>& op) = 0;
  virtual void Emit(Cast<i64, i32>& op) = 0;
  virtual void Emit(Cast<i64, u32>& op) = 0;
  virtual void Emit(Cast<i64, i64>& op) = 0;
  virtual void Emit(Cast<i64, u64>& op) = 0;
  virtual void Emit(Cast<i64, float>& op) = 0;
  virtual void Emit(Cast<i64, double>& op) = 0;
  virtual void Emit(Cast<i64, Pointer>& op) = 0;
  virtual void Emit(Cast<u8, i32>& op) = 0;
  virtual void Emit(Cast<u8, u32>& op) = 0;
  virtual void Emit(Cast<u8, i64>& op) = 0;
  virtual void Emit(Cast<u8, u64>& op) = 0;
  virtual void Emit(Cast<u8, float>& op) = 0;
  virtual void Emit(Cast<u8, double>& op) = 0;
  virtual void Emit(Cast<u8, Pointer>& op) = 0;
  virtual void Emit(Cast<u16, i32>& op) = 0;
  virtual void Emit(Cast<u16, u32>& op) = 0;
  virtual void Emit(Cast<u16, i64>& op) = 0;
  virtual void Emit(Cast<u16, u64>& op) = 0;
  virtual void Emit(Cast<u16, float>& op) = 0;
  virtual void Emit(Cast<u16, double>& op) = 0;
  virtual void Emit(Cast<u16, Pointer>& op) = 0;
  virtual void Emit(Cast<u32, i32>& op) = 0;
  virtual void Emit(Cast<u32, u32>& op) = 0;
  virtual void Emit(Cast<u32, i64>& op) = 0;
  virtual void Emit(Cast<u32, u64>& op) = 0;
  virtual void Emit(Cast<u32, float>& op) = 0;
  virtual void Emit(Cast<u32, double>& op) = 0;
  virtual void Emit(Cast<u32, Pointer>& op) = 0;
  virtual void Emit(Cast<u64, i32>& op) = 0;
  virtual void Emit(Cast<u64, u32>& op) = 0;
  virtual void Emit(Cast<u64, i64>& op) = 0;
  virtual void Emit(Cast<u64, u64>& op) = 0;
  virtual void Emit(Cast<u64, float>& op) = 0;
  virtual void Emit(Cast<u64, double>& op) = 0;
  virtual void Emit(Cast<u64, Pointer>& op) = 0;
  virtual void Emit(Cast<float, i32>& op) = 0;
  virtual void Emit(Cast<float, u32>& op) = 0;
  virtual void Emit(Cast<float, i64>& op) = 0;
  virtual void Emit(Cast<float, u64>& op) = 0;
  virtual void Emit(Cast<float, float>& op) = 0;
  virtual void Emit(Cast<float, double>& op) = 0;
  virtual void Emit(Cast<float, Pointer>& op) = 0;
  virtual void Emit(Cast<double, i32>& op) = 0;
  virtual void Emit(Cast<double, u32>& op) = 0;
  virtual void Emit(Cast<double, i64>& op) = 0;
  virtual void Emit(Cast<double, u64>& op) = 0;
  virtual void Emit(Cast<double, float>& op) = 0;
  virtual void Emit(Cast<double, double>& op) = 0;
  virtual void Emit(Cast<double, Pointer>& op) = 0;
  virtual void Emit(Cast<Pointer, i32>& op) = 0;
  virtual void Emit(Cast<Pointer, u32>& op) = 0;
  virtual void Emit(Cast<Pointer, i64>& op) = 0;
  virtual void Emit(Cast<Pointer, u64>& op) = 0;
  virtual void Emit(Cast<Pointer, float>& op) = 0;
  virtual void Emit(Cast<Pointer, double>& op) = 0;
  virtual void Emit(Cast<Pointer, Pointer>& op) = 0;
};

}