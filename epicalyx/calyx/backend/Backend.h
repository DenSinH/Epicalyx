#pragma once


#include "calyx/Calyx.h"

#include <vector>

namespace epi::calyx {

struct Backend {

  virtual void EmitProgram(const Program& program) = 0;

  virtual void Emit(const LoadLocalAddr& op) = 0;
  virtual void Emit(const LoadGlobalAddr& op) = 0;
  virtual void Emit(const ArgMakeLocal& op) = 0;
  
  virtual void Emit(const Binop<i32>& op) = 0;
  virtual void Emit(const Binop<u32>& op) = 0;
  virtual void Emit(const Binop<i64>& op) = 0;
  virtual void Emit(const Binop<u64>& op) = 0;
  virtual void Emit(const Binop<float>& op) = 0;
  virtual void Emit(const Binop<double>& op) = 0;
  virtual void Emit(const BinopImm<i32>& op) = 0;
  virtual void Emit(const BinopImm<u32>& op) = 0;
  virtual void Emit(const BinopImm<i64>& op) = 0;
  virtual void Emit(const BinopImm<u64>& op) = 0;
  virtual void Emit(const BinopImm<float>& op) = 0;
  virtual void Emit(const BinopImm<double>& op) = 0;
  virtual void Emit(const Shift<i32>& op) = 0;
  virtual void Emit(const Shift<u32>& op) = 0;
  virtual void Emit(const Shift<i64>& op) = 0;
  virtual void Emit(const Shift<u64>& op) = 0;
  virtual void Emit(const ShiftImm<i32>& op) = 0;
  virtual void Emit(const ShiftImm<u32>& op) = 0;
  virtual void Emit(const ShiftImm<i64>& op) = 0;
  virtual void Emit(const ShiftImm<u64>& op) = 0;
  virtual void Emit(const Compare<i32>& op) = 0;
  virtual void Emit(const Compare<u32>& op) = 0;
  virtual void Emit(const Compare<i64>& op) = 0;
  virtual void Emit(const Compare<u64>& op) = 0;
  virtual void Emit(const Compare<float>& op) = 0;
  virtual void Emit(const Compare<double>& op) = 0;
  virtual void Emit(const Compare<Pointer>& op) = 0;
  virtual void Emit(const CompareImm<i32>& op) = 0;
  virtual void Emit(const CompareImm<u32>& op) = 0;
  virtual void Emit(const CompareImm<i64>& op) = 0;
  virtual void Emit(const CompareImm<u64>& op) = 0;
  virtual void Emit(const CompareImm<float>& op) = 0;
  virtual void Emit(const CompareImm<double>& op) = 0;
  virtual void Emit(const CompareImm<Pointer>& op) = 0;
  virtual void Emit(const UnconditionalBranch& op) = 0;
  virtual void Emit(const BranchCompare<i32>& op) = 0;
  virtual void Emit(const BranchCompare<u32>& op) = 0;
  virtual void Emit(const BranchCompare<i64>& op) = 0;
  virtual void Emit(const BranchCompare<u64>& op) = 0;
  virtual void Emit(const BranchCompare<float>& op) = 0;
  virtual void Emit(const BranchCompare<double>& op) = 0;
  virtual void Emit(const BranchCompare<Pointer>& op) = 0;
  virtual void Emit(const BranchCompareImm<i32>& op) = 0;
  virtual void Emit(const BranchCompareImm<u32>& op) = 0;
  virtual void Emit(const BranchCompareImm<i64>& op) = 0;
  virtual void Emit(const BranchCompareImm<u64>& op) = 0;
  virtual void Emit(const BranchCompareImm<float>& op) = 0;
  virtual void Emit(const BranchCompareImm<double>& op) = 0;
  virtual void Emit(const BranchCompareImm<Pointer>& op) = 0;
  virtual void Emit(const Select& op) = 0;
  virtual void Emit(const AddToPointer<i32>& op) = 0;
  virtual void Emit(const AddToPointer<u32>& op) = 0;
  virtual void Emit(const AddToPointer<i64>& op) = 0;
  virtual void Emit(const AddToPointer<u64>& op) = 0;
  virtual void Emit(const AddToPointerImm& op) = 0;
  virtual void Emit(const Unop<i32>& op) = 0;
  virtual void Emit(const Unop<u32>& op) = 0;
  virtual void Emit(const Unop<i64>& op) = 0;
  virtual void Emit(const Unop<u64>& op) = 0;
  virtual void Emit(const Unop<float>& op) = 0;
  virtual void Emit(const Unop<double>& op) = 0;
  virtual void Emit(const Imm<i32>& op) = 0;
  virtual void Emit(const Imm<u32>& op) = 0;
  virtual void Emit(const Imm<i64>& op) = 0;
  virtual void Emit(const Imm<u64>& op) = 0;
  virtual void Emit(const Imm<float>& op) = 0;
  virtual void Emit(const Imm<double>& op) = 0;
  virtual void Emit(const LoadLocal<i8>& op) = 0;
  virtual void Emit(const LoadLocal<u8>& op) = 0;
  virtual void Emit(const LoadLocal<i16>& op) = 0;
  virtual void Emit(const LoadLocal<u16>& op) = 0;
  virtual void Emit(const LoadLocal<i32>& op) = 0;
  virtual void Emit(const LoadLocal<u32>& op) = 0;
  virtual void Emit(const LoadLocal<i64>& op) = 0;
  virtual void Emit(const LoadLocal<u64>& op) = 0;
  virtual void Emit(const LoadLocal<float>& op) = 0;
  virtual void Emit(const LoadLocal<double>& op) = 0;
  virtual void Emit(const LoadLocal<Struct>& op) = 0;
  virtual void Emit(const LoadLocal<Pointer>& op) = 0;
  virtual void Emit(const StoreLocal<i8>& op) = 0;
  virtual void Emit(const StoreLocal<u8>& op) = 0;
  virtual void Emit(const StoreLocal<i16>& op) = 0;
  virtual void Emit(const StoreLocal<u16>& op) = 0;
  virtual void Emit(const StoreLocal<i32>& op) = 0;
  virtual void Emit(const StoreLocal<u32>& op) = 0;
  virtual void Emit(const StoreLocal<i64>& op) = 0;
  virtual void Emit(const StoreLocal<u64>& op) = 0;
  virtual void Emit(const StoreLocal<float>& op) = 0;
  virtual void Emit(const StoreLocal<double>& op) = 0;
  virtual void Emit(const StoreLocal<Struct>& op) = 0;
  virtual void Emit(const StoreLocal<Pointer>& op) = 0;
  virtual void Emit(const LoadGlobal<i8>& op) = 0;
  virtual void Emit(const LoadGlobal<u8>& op) = 0;
  virtual void Emit(const LoadGlobal<i16>& op) = 0;
  virtual void Emit(const LoadGlobal<u16>& op) = 0;
  virtual void Emit(const LoadGlobal<i32>& op) = 0;
  virtual void Emit(const LoadGlobal<u32>& op) = 0;
  virtual void Emit(const LoadGlobal<i64>& op) = 0;
  virtual void Emit(const LoadGlobal<u64>& op) = 0;
  virtual void Emit(const LoadGlobal<float>& op) = 0;
  virtual void Emit(const LoadGlobal<double>& op) = 0;
  virtual void Emit(const LoadGlobal<Struct>& op) = 0;
  virtual void Emit(const LoadGlobal<Pointer>& op) = 0;
  virtual void Emit(const StoreGlobal<i8>& op) = 0;
  virtual void Emit(const StoreGlobal<u8>& op) = 0;
  virtual void Emit(const StoreGlobal<i16>& op) = 0;
  virtual void Emit(const StoreGlobal<u16>& op) = 0;
  virtual void Emit(const StoreGlobal<i32>& op) = 0;
  virtual void Emit(const StoreGlobal<u32>& op) = 0;
  virtual void Emit(const StoreGlobal<i64>& op) = 0;
  virtual void Emit(const StoreGlobal<u64>& op) = 0;
  virtual void Emit(const StoreGlobal<float>& op) = 0;
  virtual void Emit(const StoreGlobal<double>& op) = 0;
  virtual void Emit(const StoreGlobal<Struct>& op) = 0;
  virtual void Emit(const StoreGlobal<Pointer>& op) = 0;
  virtual void Emit(const LoadFromPointer<i8>& op) = 0;
  virtual void Emit(const LoadFromPointer<u8>& op) = 0;
  virtual void Emit(const LoadFromPointer<i16>& op) = 0;
  virtual void Emit(const LoadFromPointer<u16>& op) = 0;
  virtual void Emit(const LoadFromPointer<i32>& op) = 0;
  virtual void Emit(const LoadFromPointer<u32>& op) = 0;
  virtual void Emit(const LoadFromPointer<i64>& op) = 0;
  virtual void Emit(const LoadFromPointer<u64>& op) = 0;
  virtual void Emit(const LoadFromPointer<float>& op) = 0;
  virtual void Emit(const LoadFromPointer<double>& op) = 0;
  virtual void Emit(const LoadFromPointer<Struct>& op) = 0;
  virtual void Emit(const LoadFromPointer<Pointer>& op) = 0;
  virtual void Emit(const StoreToPointer<i8>& op) = 0;
  virtual void Emit(const StoreToPointer<u8>& op) = 0;
  virtual void Emit(const StoreToPointer<i16>& op) = 0;
  virtual void Emit(const StoreToPointer<u16>& op) = 0;
  virtual void Emit(const StoreToPointer<i32>& op) = 0;
  virtual void Emit(const StoreToPointer<u32>& op) = 0;
  virtual void Emit(const StoreToPointer<i64>& op) = 0;
  virtual void Emit(const StoreToPointer<u64>& op) = 0;
  virtual void Emit(const StoreToPointer<float>& op) = 0;
  virtual void Emit(const StoreToPointer<double>& op) = 0;
  virtual void Emit(const StoreToPointer<Struct>& op) = 0;
  virtual void Emit(const StoreToPointer<Pointer>& op) = 0;
  virtual void Emit(const Return<i32>& op) = 0;
  virtual void Emit(const Return<u32>& op) = 0;
  virtual void Emit(const Return<i64>& op) = 0;
  virtual void Emit(const Return<u64>& op) = 0;
  virtual void Emit(const Return<float>& op) = 0;
  virtual void Emit(const Return<double>& op) = 0;
  virtual void Emit(const Return<Struct>& op) = 0;
  virtual void Emit(const Return<Pointer>& op) = 0;
  virtual void Emit(const Return<void>& op) = 0;
  virtual void Emit(const Call<i32>& op) = 0;
  virtual void Emit(const Call<u32>& op) = 0;
  virtual void Emit(const Call<i64>& op) = 0;
  virtual void Emit(const Call<u64>& op) = 0;
  virtual void Emit(const Call<float>& op) = 0;
  virtual void Emit(const Call<double>& op) = 0;
  virtual void Emit(const Call<Struct>& op) = 0;
  virtual void Emit(const Call<Pointer>& op) = 0;
  virtual void Emit(const Call<void>& op) = 0;
  virtual void Emit(const CallLabel<i32>& op) = 0;
  virtual void Emit(const CallLabel<u32>& op) = 0;
  virtual void Emit(const CallLabel<i64>& op) = 0;
  virtual void Emit(const CallLabel<u64>& op) = 0;
  virtual void Emit(const CallLabel<float>& op) = 0;
  virtual void Emit(const CallLabel<double>& op) = 0;
  virtual void Emit(const CallLabel<Struct>& op) = 0;
  virtual void Emit(const CallLabel<Pointer>& op) = 0;
  virtual void Emit(const CallLabel<void>& op) = 0;
  virtual void Emit(const Cast<i8, i32>& op) = 0;
  virtual void Emit(const Cast<i8, u32>& op) = 0;
  virtual void Emit(const Cast<i8, i64>& op) = 0;
  virtual void Emit(const Cast<i8, u64>& op) = 0;
  virtual void Emit(const Cast<i8, float>& op) = 0;
  virtual void Emit(const Cast<i8, double>& op) = 0;
  virtual void Emit(const Cast<i8, Pointer>& op) = 0;
  virtual void Emit(const Cast<i16, i32>& op) = 0;
  virtual void Emit(const Cast<i16, u32>& op) = 0;
  virtual void Emit(const Cast<i16, i64>& op) = 0;
  virtual void Emit(const Cast<i16, u64>& op) = 0;
  virtual void Emit(const Cast<i16, float>& op) = 0;
  virtual void Emit(const Cast<i16, double>& op) = 0;
  virtual void Emit(const Cast<i16, Pointer>& op) = 0;
  virtual void Emit(const Cast<i32, i32>& op) = 0;
  virtual void Emit(const Cast<i32, u32>& op) = 0;
  virtual void Emit(const Cast<i32, i64>& op) = 0;
  virtual void Emit(const Cast<i32, u64>& op) = 0;
  virtual void Emit(const Cast<i32, float>& op) = 0;
  virtual void Emit(const Cast<i32, double>& op) = 0;
  virtual void Emit(const Cast<i32, Pointer>& op) = 0;
  virtual void Emit(const Cast<i64, i32>& op) = 0;
  virtual void Emit(const Cast<i64, u32>& op) = 0;
  virtual void Emit(const Cast<i64, i64>& op) = 0;
  virtual void Emit(const Cast<i64, u64>& op) = 0;
  virtual void Emit(const Cast<i64, float>& op) = 0;
  virtual void Emit(const Cast<i64, double>& op) = 0;
  virtual void Emit(const Cast<i64, Pointer>& op) = 0;
  virtual void Emit(const Cast<u8, i32>& op) = 0;
  virtual void Emit(const Cast<u8, u32>& op) = 0;
  virtual void Emit(const Cast<u8, i64>& op) = 0;
  virtual void Emit(const Cast<u8, u64>& op) = 0;
  virtual void Emit(const Cast<u8, float>& op) = 0;
  virtual void Emit(const Cast<u8, double>& op) = 0;
  virtual void Emit(const Cast<u8, Pointer>& op) = 0;
  virtual void Emit(const Cast<u16, i32>& op) = 0;
  virtual void Emit(const Cast<u16, u32>& op) = 0;
  virtual void Emit(const Cast<u16, i64>& op) = 0;
  virtual void Emit(const Cast<u16, u64>& op) = 0;
  virtual void Emit(const Cast<u16, float>& op) = 0;
  virtual void Emit(const Cast<u16, double>& op) = 0;
  virtual void Emit(const Cast<u16, Pointer>& op) = 0;
  virtual void Emit(const Cast<u32, i32>& op) = 0;
  virtual void Emit(const Cast<u32, u32>& op) = 0;
  virtual void Emit(const Cast<u32, i64>& op) = 0;
  virtual void Emit(const Cast<u32, u64>& op) = 0;
  virtual void Emit(const Cast<u32, float>& op) = 0;
  virtual void Emit(const Cast<u32, double>& op) = 0;
  virtual void Emit(const Cast<u32, Pointer>& op) = 0;
  virtual void Emit(const Cast<u64, i32>& op) = 0;
  virtual void Emit(const Cast<u64, u32>& op) = 0;
  virtual void Emit(const Cast<u64, i64>& op) = 0;
  virtual void Emit(const Cast<u64, u64>& op) = 0;
  virtual void Emit(const Cast<u64, float>& op) = 0;
  virtual void Emit(const Cast<u64, double>& op) = 0;
  virtual void Emit(const Cast<u64, Pointer>& op) = 0;
  virtual void Emit(const Cast<float, i32>& op) = 0;
  virtual void Emit(const Cast<float, u32>& op) = 0;
  virtual void Emit(const Cast<float, i64>& op) = 0;
  virtual void Emit(const Cast<float, u64>& op) = 0;
  virtual void Emit(const Cast<float, float>& op) = 0;
  virtual void Emit(const Cast<float, double>& op) = 0;
  virtual void Emit(const Cast<float, Pointer>& op) = 0;
  virtual void Emit(const Cast<double, i32>& op) = 0;
  virtual void Emit(const Cast<double, u32>& op) = 0;
  virtual void Emit(const Cast<double, i64>& op) = 0;
  virtual void Emit(const Cast<double, u64>& op) = 0;
  virtual void Emit(const Cast<double, float>& op) = 0;
  virtual void Emit(const Cast<double, double>& op) = 0;
  virtual void Emit(const Cast<double, Pointer>& op) = 0;
  virtual void Emit(const Cast<Pointer, i32>& op) = 0;
  virtual void Emit(const Cast<Pointer, u32>& op) = 0;
  virtual void Emit(const Cast<Pointer, i64>& op) = 0;
  virtual void Emit(const Cast<Pointer, u64>& op) = 0;
  virtual void Emit(const Cast<Pointer, float>& op) = 0;
  virtual void Emit(const Cast<Pointer, double>& op) = 0;
  virtual void Emit(const Cast<Pointer, Pointer>& op) = 0;
};

}