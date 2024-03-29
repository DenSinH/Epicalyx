#pragma once


#include "calyx/Calyx.h"

#include <vector>

namespace epi::calyx {

struct Backend {

  virtual void EmitProgram(Program& program) = 0;

  virtual void Emit(AllocateLocal& op) = 0;
  virtual void Emit(DeallocateLocal& op) = 0;
  virtual void Emit(LoadLocalAddr& op) = 0;
  virtual void Emit(LoadGlobalAddr& op) = 0;
  virtual void Emit(ArgMakeLocal& op) = 0;

  virtual void Emit(Binop<i32>& op) = 0;
  virtual void Emit(Binop<u32>& op) = 0;
  virtual void Emit(Binop<i64>& op) = 0;
  virtual void Emit(Binop<u64>& op) = 0;
  virtual void Emit(Binop<float>& op) = 0;
  virtual void Emit(Binop<double>& op) = 0;
  virtual void Emit(BinopImm<i32>& op) = 0;
  virtual void Emit(BinopImm<u32>& op) = 0;
  virtual void Emit(BinopImm<i64>& op) = 0;
  virtual void Emit(BinopImm<u64>& op) = 0;
  virtual void Emit(BinopImm<float>& op) = 0;
  virtual void Emit(BinopImm<double>& op) = 0;
  virtual void Emit(Shift<i32>& op) = 0;
  virtual void Emit(Shift<u32>& op) = 0;
  virtual void Emit(Shift<i64>& op) = 0;
  virtual void Emit(Shift<u64>& op) = 0;
  virtual void Emit(ShiftImm<i32>& op) = 0;
  virtual void Emit(ShiftImm<u32>& op) = 0;
  virtual void Emit(ShiftImm<i64>& op) = 0;
  virtual void Emit(ShiftImm<u64>& op) = 0;
  virtual void Emit(Compare<i32>& op) = 0;
  virtual void Emit(Compare<u32>& op) = 0;
  virtual void Emit(Compare<i64>& op) = 0;
  virtual void Emit(Compare<u64>& op) = 0;
  virtual void Emit(Compare<float>& op) = 0;
  virtual void Emit(Compare<double>& op) = 0;
  virtual void Emit(Compare<Pointer>& op) = 0;
  virtual void Emit(CompareImm<i32>& op) = 0;
  virtual void Emit(CompareImm<u32>& op) = 0;
  virtual void Emit(CompareImm<i64>& op) = 0;
  virtual void Emit(CompareImm<u64>& op) = 0;
  virtual void Emit(CompareImm<float>& op) = 0;
  virtual void Emit(CompareImm<double>& op) = 0;
  virtual void Emit(CompareImm<Pointer>& op) = 0;
  virtual void Emit(UnconditionalBranch& op) = 0;
  virtual void Emit(BranchCompare<i32>& op) = 0;
  virtual void Emit(BranchCompare<u32>& op) = 0;
  virtual void Emit(BranchCompare<i64>& op) = 0;
  virtual void Emit(BranchCompare<u64>& op) = 0;
  virtual void Emit(BranchCompare<float>& op) = 0;
  virtual void Emit(BranchCompare<double>& op) = 0;
  virtual void Emit(BranchCompare<Pointer>& op) = 0;
  virtual void Emit(BranchCompareImm<i32>& op) = 0;
  virtual void Emit(BranchCompareImm<u32>& op) = 0;
  virtual void Emit(BranchCompareImm<i64>& op) = 0;
  virtual void Emit(BranchCompareImm<u64>& op) = 0;
  virtual void Emit(BranchCompareImm<float>& op) = 0;
  virtual void Emit(BranchCompareImm<double>& op) = 0;
  virtual void Emit(BranchCompareImm<Pointer>& op) = 0;
  virtual void Emit(Select& op) = 0;
  virtual void Emit(AddToPointer<i32>& op) = 0;
  virtual void Emit(AddToPointer<u32>& op) = 0;
  virtual void Emit(AddToPointer<i64>& op) = 0;
  virtual void Emit(AddToPointer<u64>& op) = 0;
  virtual void Emit(AddToPointerImm& op) = 0;
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
  virtual void Emit(LoadLocal<i8>& op) = 0;
  virtual void Emit(LoadLocal<u8>& op) = 0;
  virtual void Emit(LoadLocal<i16>& op) = 0;
  virtual void Emit(LoadLocal<u16>& op) = 0;
  virtual void Emit(LoadLocal<i32>& op) = 0;
  virtual void Emit(LoadLocal<u32>& op) = 0;
  virtual void Emit(LoadLocal<i64>& op) = 0;
  virtual void Emit(LoadLocal<u64>& op) = 0;
  virtual void Emit(LoadLocal<float>& op) = 0;
  virtual void Emit(LoadLocal<double>& op) = 0;
  virtual void Emit(LoadLocal<Struct>& op) = 0;
  virtual void Emit(LoadLocal<Pointer>& op) = 0;
  virtual void Emit(StoreLocal<i8>& op) = 0;
  virtual void Emit(StoreLocal<u8>& op) = 0;
  virtual void Emit(StoreLocal<i16>& op) = 0;
  virtual void Emit(StoreLocal<u16>& op) = 0;
  virtual void Emit(StoreLocal<i32>& op) = 0;
  virtual void Emit(StoreLocal<u32>& op) = 0;
  virtual void Emit(StoreLocal<i64>& op) = 0;
  virtual void Emit(StoreLocal<u64>& op) = 0;
  virtual void Emit(StoreLocal<float>& op) = 0;
  virtual void Emit(StoreLocal<double>& op) = 0;
  virtual void Emit(StoreLocal<Struct>& op) = 0;
  virtual void Emit(StoreLocal<Pointer>& op) = 0;
  virtual void Emit(LoadGlobal<i8>& op) = 0;
  virtual void Emit(LoadGlobal<u8>& op) = 0;
  virtual void Emit(LoadGlobal<i16>& op) = 0;
  virtual void Emit(LoadGlobal<u16>& op) = 0;
  virtual void Emit(LoadGlobal<i32>& op) = 0;
  virtual void Emit(LoadGlobal<u32>& op) = 0;
  virtual void Emit(LoadGlobal<i64>& op) = 0;
  virtual void Emit(LoadGlobal<u64>& op) = 0;
  virtual void Emit(LoadGlobal<float>& op) = 0;
  virtual void Emit(LoadGlobal<double>& op) = 0;
  virtual void Emit(LoadGlobal<Struct>& op) = 0;
  virtual void Emit(LoadGlobal<Pointer>& op) = 0;
  virtual void Emit(StoreGlobal<i8>& op) = 0;
  virtual void Emit(StoreGlobal<u8>& op) = 0;
  virtual void Emit(StoreGlobal<i16>& op) = 0;
  virtual void Emit(StoreGlobal<u16>& op) = 0;
  virtual void Emit(StoreGlobal<i32>& op) = 0;
  virtual void Emit(StoreGlobal<u32>& op) = 0;
  virtual void Emit(StoreGlobal<i64>& op) = 0;
  virtual void Emit(StoreGlobal<u64>& op) = 0;
  virtual void Emit(StoreGlobal<float>& op) = 0;
  virtual void Emit(StoreGlobal<double>& op) = 0;
  virtual void Emit(StoreGlobal<Struct>& op) = 0;
  virtual void Emit(StoreGlobal<Pointer>& op) = 0;
  virtual void Emit(LoadFromPointer<i8>& op) = 0;
  virtual void Emit(LoadFromPointer<u8>& op) = 0;
  virtual void Emit(LoadFromPointer<i16>& op) = 0;
  virtual void Emit(LoadFromPointer<u16>& op) = 0;
  virtual void Emit(LoadFromPointer<i32>& op) = 0;
  virtual void Emit(LoadFromPointer<u32>& op) = 0;
  virtual void Emit(LoadFromPointer<i64>& op) = 0;
  virtual void Emit(LoadFromPointer<u64>& op) = 0;
  virtual void Emit(LoadFromPointer<float>& op) = 0;
  virtual void Emit(LoadFromPointer<double>& op) = 0;
  virtual void Emit(LoadFromPointer<Struct>& op) = 0;
  virtual void Emit(LoadFromPointer<Pointer>& op) = 0;
  virtual void Emit(StoreToPointer<i8>& op) = 0;
  virtual void Emit(StoreToPointer<u8>& op) = 0;
  virtual void Emit(StoreToPointer<i16>& op) = 0;
  virtual void Emit(StoreToPointer<u16>& op) = 0;
  virtual void Emit(StoreToPointer<i32>& op) = 0;
  virtual void Emit(StoreToPointer<u32>& op) = 0;
  virtual void Emit(StoreToPointer<i64>& op) = 0;
  virtual void Emit(StoreToPointer<u64>& op) = 0;
  virtual void Emit(StoreToPointer<float>& op) = 0;
  virtual void Emit(StoreToPointer<double>& op) = 0;
  virtual void Emit(StoreToPointer<Struct>& op) = 0;
  virtual void Emit(StoreToPointer<Pointer>& op) = 0;
  virtual void Emit(Return<i32>& op) = 0;
  virtual void Emit(Return<u32>& op) = 0;
  virtual void Emit(Return<i64>& op) = 0;
  virtual void Emit(Return<u64>& op) = 0;
  virtual void Emit(Return<float>& op) = 0;
  virtual void Emit(Return<double>& op) = 0;
  virtual void Emit(Return<Struct>& op) = 0;
  virtual void Emit(Return<Pointer>& op) = 0;
  virtual void Emit(Return<void>& op) = 0;
  virtual void Emit(Call<i32>& op) = 0;
  virtual void Emit(Call<u32>& op) = 0;
  virtual void Emit(Call<i64>& op) = 0;
  virtual void Emit(Call<u64>& op) = 0;
  virtual void Emit(Call<float>& op) = 0;
  virtual void Emit(Call<double>& op) = 0;
  virtual void Emit(Call<Struct>& op) = 0;
  virtual void Emit(Call<Pointer>& op) = 0;
  virtual void Emit(Call<void>& op) = 0;
  virtual void Emit(CallLabel<i32>& op) = 0;
  virtual void Emit(CallLabel<u32>& op) = 0;
  virtual void Emit(CallLabel<i64>& op) = 0;
  virtual void Emit(CallLabel<u64>& op) = 0;
  virtual void Emit(CallLabel<float>& op) = 0;
  virtual void Emit(CallLabel<double>& op) = 0;
  virtual void Emit(CallLabel<Struct>& op) = 0;
  virtual void Emit(CallLabel<Pointer>& op) = 0;
  virtual void Emit(CallLabel<void>& op) = 0;
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