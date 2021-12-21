#pragma once

#include "calyx/backend/Backend.h"

#include <vector>
#include <unordered_map>

namespace epi {

using namespace calyx;

// todo: don't do this

template<typename T>
struct IRCompare : calyx::Backend {
  bool result;

  void EmitProgram(Program& program) final { }

  void Emit(AllocateLocal& op) final { result = std::is_same_v<AllocateLocal, T>; }
  void Emit(DeallocateLocal& op) final { result = std::is_same_v<DeallocateLocal, T>; }
  void Emit(LoadLocalAddr& op) final { result = std::is_same_v<LoadLocalAddr, T>; }
  void Emit(LoadGlobalAddr& op) final { result = std::is_same_v<LoadGlobalAddr, T>; }
  void Emit(ArgMakeLocal& op) final { result = std::is_same_v<ArgMakeLocal, T>; }

  void Emit(Binop<i32>& op) final { result = std::is_same_v<Binop<i32>, T>; }
  void Emit(Binop<u32>& op) final { result = std::is_same_v<Binop<u32>, T>; }
  void Emit(Binop<i64>& op) final { result = std::is_same_v<Binop<i64>, T>; }
  void Emit(Binop<u64>& op) final { result = std::is_same_v<Binop<u64>, T>; }
  void Emit(Binop<float>& op) final { result = std::is_same_v<Binop<float>, T>; }
  void Emit(Binop<double>& op) final { result = std::is_same_v<Binop<double>, T>; }
  void Emit(BinopImm<i32>& op) final { result = std::is_same_v<BinopImm<i32>, T>; }
  void Emit(BinopImm<u32>& op) final { result = std::is_same_v<BinopImm<u32>, T>; }
  void Emit(BinopImm<i64>& op) final { result = std::is_same_v<BinopImm<i64>, T>; }
  void Emit(BinopImm<u64>& op) final { result = std::is_same_v<BinopImm<u64>, T>; }
  void Emit(BinopImm<float>& op) final { result = std::is_same_v<BinopImm<float>, T>; }
  void Emit(BinopImm<double>& op) final { result = std::is_same_v<BinopImm<double>, T>; }
  void Emit(Shift<i32>& op) final { result = std::is_same_v<Shift<i32>, T>; }
  void Emit(Shift<u32>& op) final { result = std::is_same_v<Shift<u32>, T>; }
  void Emit(Shift<i64>& op) final { result = std::is_same_v<Shift<i64>, T>; }
  void Emit(Shift<u64>& op) final { result = std::is_same_v<Shift<u64>, T>; }
  void Emit(ShiftImm<i32>& op) final { result = std::is_same_v<ShiftImm<i32>, T>; }
  void Emit(ShiftImm<u32>& op) final { result = std::is_same_v<ShiftImm<u32>, T>; }
  void Emit(ShiftImm<i64>& op) final { result = std::is_same_v<ShiftImm<i64>, T>; }
  void Emit(ShiftImm<u64>& op) final { result = std::is_same_v<ShiftImm<u64>, T>; }
  void Emit(Compare<i32>& op) final { result = std::is_same_v<Compare<i32>, T>; }
  void Emit(Compare<u32>& op) final { result = std::is_same_v<Compare<u32>, T>; }
  void Emit(Compare<i64>& op) final { result = std::is_same_v<Compare<i64>, T>; }
  void Emit(Compare<u64>& op) final { result = std::is_same_v<Compare<u64>, T>; }
  void Emit(Compare<float>& op) final { result = std::is_same_v<Compare<float>, T>; }
  void Emit(Compare<double>& op) final { result = std::is_same_v<Compare<double>, T>; }
  void Emit(Compare<Pointer>& op) final { result = std::is_same_v<Compare<Pointer>, T>; }
  void Emit(CompareImm<i32>& op) final { result = std::is_same_v<CompareImm<i32>, T>; }
  void Emit(CompareImm<u32>& op) final { result = std::is_same_v<CompareImm<u32>, T>; }
  void Emit(CompareImm<i64>& op) final { result = std::is_same_v<CompareImm<i64>, T>; }
  void Emit(CompareImm<u64>& op) final { result = std::is_same_v<CompareImm<u64>, T>; }
  void Emit(CompareImm<float>& op) final { result = std::is_same_v<CompareImm<float>, T>; }
  void Emit(CompareImm<double>& op) final { result = std::is_same_v<CompareImm<double>, T>; }
  void Emit(CompareImm<Pointer>& op) final { result = std::is_same_v<CompareImm<Pointer>, T>; }
  void Emit(UnconditionalBranch& op) final { result = std::is_same_v<UnconditionalBranch, T>; }
  void Emit(BranchCompare<i32>& op) final { result = std::is_same_v<BranchCompare<i32>, T>; }
  void Emit(BranchCompare<u32>& op) final { result = std::is_same_v<BranchCompare<u32>, T>; }
  void Emit(BranchCompare<i64>& op) final { result = std::is_same_v<BranchCompare<i64>, T>; }
  void Emit(BranchCompare<u64>& op) final { result = std::is_same_v<BranchCompare<u64>, T>; }
  void Emit(BranchCompare<float>& op) final { result = std::is_same_v<BranchCompare<float>, T>; }
  void Emit(BranchCompare<double>& op) final { result = std::is_same_v<BranchCompare<double>, T>; }
  void Emit(BranchCompare<Pointer>& op) final { result = std::is_same_v<BranchCompare<Pointer>, T>; }
  void Emit(BranchCompareImm<i32>& op) final { result = std::is_same_v<BranchCompareImm<i32>, T>; }
  void Emit(BranchCompareImm<u32>& op) final { result = std::is_same_v<BranchCompareImm<u32>, T>; }
  void Emit(BranchCompareImm<i64>& op) final { result = std::is_same_v<BranchCompareImm<i64>, T>; }
  void Emit(BranchCompareImm<u64>& op) final { result = std::is_same_v<BranchCompareImm<u64>, T>; }
  void Emit(BranchCompareImm<float>& op) final { result = std::is_same_v<BranchCompareImm<float>, T>; }
  void Emit(BranchCompareImm<double>& op) final { result = std::is_same_v<BranchCompareImm<double>, T>; }
  void Emit(BranchCompareImm<Pointer>& op) final { result = std::is_same_v<BranchCompareImm<Pointer>, T>; }
  void Emit(Select& op) final { result = std::is_same_v<Select, T>; }
  void Emit(AddToPointer<i32>& op) final { result = std::is_same_v<AddToPointer<i32>, T>; }
  void Emit(AddToPointer<u32>& op) final { result = std::is_same_v<AddToPointer<u32>, T>; }
  void Emit(AddToPointer<i64>& op) final { result = std::is_same_v<AddToPointer<i64>, T>; }
  void Emit(AddToPointer<u64>& op) final { result = std::is_same_v<AddToPointer<u64>, T>; }
  void Emit(AddToPointerImm& op) final { result = std::is_same_v<AddToPointerImm, T>; }
  void Emit(Unop<i32>& op) final { result = std::is_same_v<Unop<i32>, T>; }
  void Emit(Unop<u32>& op) final { result = std::is_same_v<Unop<u32>, T>; }
  void Emit(Unop<i64>& op) final { result = std::is_same_v<Unop<i64>, T>; }
  void Emit(Unop<u64>& op) final { result = std::is_same_v<Unop<u64>, T>; }
  void Emit(Unop<float>& op) final { result = std::is_same_v<Unop<float>, T>; }
  void Emit(Unop<double>& op) final { result = std::is_same_v<Unop<double>, T>; }
  void Emit(Imm<i32>& op) final { result = std::is_same_v<Imm<i32>, T>; }
  void Emit(Imm<u32>& op) final { result = std::is_same_v<Imm<u32>, T>; }
  void Emit(Imm<i64>& op) final { result = std::is_same_v<Imm<i64>, T>; }
  void Emit(Imm<u64>& op) final { result = std::is_same_v<Imm<u64>, T>; }
  void Emit(Imm<float>& op) final { result = std::is_same_v<Imm<float>, T>; }
  void Emit(Imm<double>& op) final { result = std::is_same_v<Imm<double>, T>; }
  void Emit(LoadLocal<i8>& op) final { result = std::is_same_v<LoadLocal<i8>, T>; }
  void Emit(LoadLocal<u8>& op) final { result = std::is_same_v<LoadLocal<u8>, T>; }
  void Emit(LoadLocal<i16>& op) final { result = std::is_same_v<LoadLocal<i16>, T>; }
  void Emit(LoadLocal<u16>& op) final { result = std::is_same_v<LoadLocal<u16>, T>; }
  void Emit(LoadLocal<i32>& op) final { result = std::is_same_v<LoadLocal<i32>, T>; }
  void Emit(LoadLocal<u32>& op) final { result = std::is_same_v<LoadLocal<u32>, T>; }
  void Emit(LoadLocal<i64>& op) final { result = std::is_same_v<LoadLocal<i64>, T>; }
  void Emit(LoadLocal<u64>& op) final { result = std::is_same_v<LoadLocal<u64>, T>; }
  void Emit(LoadLocal<float>& op) final { result = std::is_same_v<LoadLocal<float>, T>; }
  void Emit(LoadLocal<double>& op) final { result = std::is_same_v<LoadLocal<double>, T>; }
  void Emit(LoadLocal<Struct>& op) final { result = std::is_same_v<LoadLocal<Struct>, T>; }
  void Emit(LoadLocal<Pointer>& op) final { result = std::is_same_v<LoadLocal<Pointer>, T>; }
  void Emit(StoreLocal<i8>& op) final { result = std::is_same_v<StoreLocal<i8>, T>; }
  void Emit(StoreLocal<u8>& op) final { result = std::is_same_v<StoreLocal<u8>, T>; }
  void Emit(StoreLocal<i16>& op) final { result = std::is_same_v<StoreLocal<i16>, T>; }
  void Emit(StoreLocal<u16>& op) final { result = std::is_same_v<StoreLocal<u16>, T>; }
  void Emit(StoreLocal<i32>& op) final { result = std::is_same_v<StoreLocal<i32>, T>; }
  void Emit(StoreLocal<u32>& op) final { result = std::is_same_v<StoreLocal<u32>, T>; }
  void Emit(StoreLocal<i64>& op) final { result = std::is_same_v<StoreLocal<i64>, T>; }
  void Emit(StoreLocal<u64>& op) final { result = std::is_same_v<StoreLocal<u64>, T>; }
  void Emit(StoreLocal<float>& op) final { result = std::is_same_v<StoreLocal<float>, T>; }
  void Emit(StoreLocal<double>& op) final { result = std::is_same_v<StoreLocal<double>, T>; }
  void Emit(StoreLocal<Struct>& op) final { result = std::is_same_v<StoreLocal<Struct>, T>; }
  void Emit(StoreLocal<Pointer>& op) final { result = std::is_same_v<StoreLocal<Pointer>, T>; }
  void Emit(LoadGlobal<i8>& op) final { result = std::is_same_v<LoadGlobal<i8>, T>; }
  void Emit(LoadGlobal<u8>& op) final { result = std::is_same_v<LoadGlobal<u8>, T>; }
  void Emit(LoadGlobal<i16>& op) final { result = std::is_same_v<LoadGlobal<i16>, T>; }
  void Emit(LoadGlobal<u16>& op) final { result = std::is_same_v<LoadGlobal<u16>, T>; }
  void Emit(LoadGlobal<i32>& op) final { result = std::is_same_v<LoadGlobal<i32>, T>; }
  void Emit(LoadGlobal<u32>& op) final { result = std::is_same_v<LoadGlobal<u32>, T>; }
  void Emit(LoadGlobal<i64>& op) final { result = std::is_same_v<LoadGlobal<i64>, T>; }
  void Emit(LoadGlobal<u64>& op) final { result = std::is_same_v<LoadGlobal<u64>, T>; }
  void Emit(LoadGlobal<float>& op) final { result = std::is_same_v<LoadGlobal<float>, T>; }
  void Emit(LoadGlobal<double>& op) final { result = std::is_same_v<LoadGlobal<double>, T>; }
  void Emit(LoadGlobal<Struct>& op) final { result = std::is_same_v<LoadGlobal<Struct>, T>; }
  void Emit(LoadGlobal<Pointer>& op) final { result = std::is_same_v<LoadGlobal<Pointer>, T>; }
  void Emit(StoreGlobal<i8>& op) final { result = std::is_same_v<StoreGlobal<i8>, T>; }
  void Emit(StoreGlobal<u8>& op) final { result = std::is_same_v<StoreGlobal<u8>, T>; }
  void Emit(StoreGlobal<i16>& op) final { result = std::is_same_v<StoreGlobal<i16>, T>; }
  void Emit(StoreGlobal<u16>& op) final { result = std::is_same_v<StoreGlobal<u16>, T>; }
  void Emit(StoreGlobal<i32>& op) final { result = std::is_same_v<StoreGlobal<i32>, T>; }
  void Emit(StoreGlobal<u32>& op) final { result = std::is_same_v<StoreGlobal<u32>, T>; }
  void Emit(StoreGlobal<i64>& op) final { result = std::is_same_v<StoreGlobal<i64>, T>; }
  void Emit(StoreGlobal<u64>& op) final { result = std::is_same_v<StoreGlobal<u64>, T>; }
  void Emit(StoreGlobal<float>& op) final { result = std::is_same_v<StoreGlobal<float>, T>; }
  void Emit(StoreGlobal<double>& op) final { result = std::is_same_v<StoreGlobal<double>, T>; }
  void Emit(StoreGlobal<Struct>& op) final { result = std::is_same_v<StoreGlobal<Struct>, T>; }
  void Emit(StoreGlobal<Pointer>& op) final { result = std::is_same_v<StoreGlobal<Pointer>, T>; }
  void Emit(LoadFromPointer<i8>& op) final { result = std::is_same_v<LoadFromPointer<i8>, T>; }
  void Emit(LoadFromPointer<u8>& op) final { result = std::is_same_v<LoadFromPointer<u8>, T>; }
  void Emit(LoadFromPointer<i16>& op) final { result = std::is_same_v<LoadFromPointer<i16>, T>; }
  void Emit(LoadFromPointer<u16>& op) final { result = std::is_same_v<LoadFromPointer<u16>, T>; }
  void Emit(LoadFromPointer<i32>& op) final { result = std::is_same_v<LoadFromPointer<i32>, T>; }
  void Emit(LoadFromPointer<u32>& op) final { result = std::is_same_v<LoadFromPointer<u32>, T>; }
  void Emit(LoadFromPointer<i64>& op) final { result = std::is_same_v<LoadFromPointer<i64>, T>; }
  void Emit(LoadFromPointer<u64>& op) final { result = std::is_same_v<LoadFromPointer<u64>, T>; }
  void Emit(LoadFromPointer<float>& op) final { result = std::is_same_v<LoadFromPointer<float>, T>; }
  void Emit(LoadFromPointer<double>& op) final { result = std::is_same_v<LoadFromPointer<double>, T>; }
  void Emit(LoadFromPointer<Struct>& op) final { result = std::is_same_v<LoadFromPointer<Struct>, T>; }
  void Emit(LoadFromPointer<Pointer>& op) final { result = std::is_same_v<LoadFromPointer<Pointer>, T>; }
  void Emit(StoreToPointer<i8>& op) final { result = std::is_same_v<StoreToPointer<i8>, T>; }
  void Emit(StoreToPointer<u8>& op) final { result = std::is_same_v<StoreToPointer<u8>, T>; }
  void Emit(StoreToPointer<i16>& op) final { result = std::is_same_v<StoreToPointer<i16>, T>; }
  void Emit(StoreToPointer<u16>& op) final { result = std::is_same_v<StoreToPointer<u16>, T>; }
  void Emit(StoreToPointer<i32>& op) final { result = std::is_same_v<StoreToPointer<i32>, T>; }
  void Emit(StoreToPointer<u32>& op) final { result = std::is_same_v<StoreToPointer<u32>, T>; }
  void Emit(StoreToPointer<i64>& op) final { result = std::is_same_v<StoreToPointer<i64>, T>; }
  void Emit(StoreToPointer<u64>& op) final { result = std::is_same_v<StoreToPointer<u64>, T>; }
  void Emit(StoreToPointer<float>& op) final { result = std::is_same_v<StoreToPointer<float>, T>; }
  void Emit(StoreToPointer<double>& op) final { result = std::is_same_v<StoreToPointer<double>, T>; }
  void Emit(StoreToPointer<Struct>& op) final { result = std::is_same_v<StoreToPointer<Struct>, T>; }
  void Emit(StoreToPointer<Pointer>& op) final { result = std::is_same_v<StoreToPointer<Pointer>, T>; }
  void Emit(Return<i32>& op) final { result = std::is_same_v<Return<i32>, T>; }
  void Emit(Return<u32>& op) final { result = std::is_same_v<Return<u32>, T>; }
  void Emit(Return<i64>& op) final { result = std::is_same_v<Return<i64>, T>; }
  void Emit(Return<u64>& op) final { result = std::is_same_v<Return<u64>, T>; }
  void Emit(Return<float>& op) final { result = std::is_same_v<Return<float>, T>; }
  void Emit(Return<double>& op) final { result = std::is_same_v<Return<double>, T>; }
  void Emit(Return<Struct>& op) final { result = std::is_same_v<Return<Struct>, T>; }
  void Emit(Return<Pointer>& op) final { result = std::is_same_v<Return<Pointer>, T>; }
  void Emit(Return<void>& op) final { result = std::is_same_v<Return<void>, T>; }
  void Emit(Call<i32>& op) final { result = std::is_same_v<Call<i32>, T>; }
  void Emit(Call<u32>& op) final { result = std::is_same_v<Call<u32>, T>; }
  void Emit(Call<i64>& op) final { result = std::is_same_v<Call<i64>, T>; }
  void Emit(Call<u64>& op) final { result = std::is_same_v<Call<u64>, T>; }
  void Emit(Call<float>& op) final { result = std::is_same_v<Call<float>, T>; }
  void Emit(Call<double>& op) final { result = std::is_same_v<Call<double>, T>; }
  void Emit(Call<Struct>& op) final { result = std::is_same_v<Call<Struct>, T>; }
  void Emit(Call<Pointer>& op) final { result = std::is_same_v<Call<Pointer>, T>; }
  void Emit(Call<void>& op) final { result = std::is_same_v<Call<void>, T>; }
  void Emit(CallLabel<i32>& op) final { result = std::is_same_v<CallLabel<i32>, T>; }
  void Emit(CallLabel<u32>& op) final { result = std::is_same_v<CallLabel<u32>, T>; }
  void Emit(CallLabel<i64>& op) final { result = std::is_same_v<CallLabel<i64>, T>; }
  void Emit(CallLabel<u64>& op) final { result = std::is_same_v<CallLabel<u64>, T>; }
  void Emit(CallLabel<float>& op) final { result = std::is_same_v<CallLabel<float>, T>; }
  void Emit(CallLabel<double>& op) final { result = std::is_same_v<CallLabel<double>, T>; }
  void Emit(CallLabel<Struct>& op) final { result = std::is_same_v<CallLabel<Struct>, T>; }
  void Emit(CallLabel<Pointer>& op) final { result = std::is_same_v<CallLabel<Pointer>, T>; }
  void Emit(CallLabel<void>& op) final { result = std::is_same_v<CallLabel<void>, T>; }
  void Emit(Cast<i8, i32>& op) final { result = std::is_same_v<Cast<i8, i32>, T>; }
  void Emit(Cast<i8, u32>& op) final { result = std::is_same_v<Cast<i8, u32>, T>; }
  void Emit(Cast<i8, i64>& op) final { result = std::is_same_v<Cast<i8, i64>, T>; }
  void Emit(Cast<i8, u64>& op) final { result = std::is_same_v<Cast<i8, u64>, T>; }
  void Emit(Cast<i8, float>& op) final { result = std::is_same_v<Cast<i8, float>, T>; }
  void Emit(Cast<i8, double>& op) final { result = std::is_same_v<Cast<i8, double>, T>; }
  void Emit(Cast<i8, Pointer>& op) final { result = std::is_same_v<Cast<i8, Pointer>, T>; }
  void Emit(Cast<i16, i32>& op) final { result = std::is_same_v<Cast<i16, i32>, T>; }
  void Emit(Cast<i16, u32>& op) final { result = std::is_same_v<Cast<i16, u32>, T>; }
  void Emit(Cast<i16, i64>& op) final { result = std::is_same_v<Cast<i16, i64>, T>; }
  void Emit(Cast<i16, u64>& op) final { result = std::is_same_v<Cast<i16, u64>, T>; }
  void Emit(Cast<i16, float>& op) final { result = std::is_same_v<Cast<i16, float>, T>; }
  void Emit(Cast<i16, double>& op) final { result = std::is_same_v<Cast<i16, double>, T>; }
  void Emit(Cast<i16, Pointer>& op) final { result = std::is_same_v<Cast<i16, Pointer>, T>; }
  void Emit(Cast<i32, i32>& op) final { result = std::is_same_v<Cast<i32, i32>, T>; }
  void Emit(Cast<i32, u32>& op) final { result = std::is_same_v<Cast<i32, u32>, T>; }
  void Emit(Cast<i32, i64>& op) final { result = std::is_same_v<Cast<i32, i64>, T>; }
  void Emit(Cast<i32, u64>& op) final { result = std::is_same_v<Cast<i32, u64>, T>; }
  void Emit(Cast<i32, float>& op) final { result = std::is_same_v<Cast<i32, float>, T>; }
  void Emit(Cast<i32, double>& op) final { result = std::is_same_v<Cast<i32, double>, T>; }
  void Emit(Cast<i32, Pointer>& op) final { result = std::is_same_v<Cast<i32, Pointer>, T>; }
  void Emit(Cast<i64, i32>& op) final { result = std::is_same_v<Cast<i64, i32>, T>; }
  void Emit(Cast<i64, u32>& op) final { result = std::is_same_v<Cast<i64, u32>, T>; }
  void Emit(Cast<i64, i64>& op) final { result = std::is_same_v<Cast<i64, i64>, T>; }
  void Emit(Cast<i64, u64>& op) final { result = std::is_same_v<Cast<i64, u64>, T>; }
  void Emit(Cast<i64, float>& op) final { result = std::is_same_v<Cast<i64, float>, T>; }
  void Emit(Cast<i64, double>& op) final { result = std::is_same_v<Cast<i64, double>, T>; }
  void Emit(Cast<i64, Pointer>& op) final { result = std::is_same_v<Cast<i64, Pointer>, T>; }
  void Emit(Cast<u8, i32>& op) final { result = std::is_same_v<Cast<u8, i32>, T>; }
  void Emit(Cast<u8, u32>& op) final { result = std::is_same_v<Cast<u8, u32>, T>; }
  void Emit(Cast<u8, i64>& op) final { result = std::is_same_v<Cast<u8, i64>, T>; }
  void Emit(Cast<u8, u64>& op) final { result = std::is_same_v<Cast<u8, u64>, T>; }
  void Emit(Cast<u8, float>& op) final { result = std::is_same_v<Cast<u8, float>, T>; }
  void Emit(Cast<u8, double>& op) final { result = std::is_same_v<Cast<u8, double>, T>; }
  void Emit(Cast<u8, Pointer>& op) final { result = std::is_same_v<Cast<u8, Pointer>, T>; }
  void Emit(Cast<u16, i32>& op) final { result = std::is_same_v<Cast<u16, i32>, T>; }
  void Emit(Cast<u16, u32>& op) final { result = std::is_same_v<Cast<u16, u32>, T>; }
  void Emit(Cast<u16, i64>& op) final { result = std::is_same_v<Cast<u16, i64>, T>; }
  void Emit(Cast<u16, u64>& op) final { result = std::is_same_v<Cast<u16, u64>, T>; }
  void Emit(Cast<u16, float>& op) final { result = std::is_same_v<Cast<u16, float>, T>; }
  void Emit(Cast<u16, double>& op) final { result = std::is_same_v<Cast<u16, double>, T>; }
  void Emit(Cast<u16, Pointer>& op) final { result = std::is_same_v<Cast<u16, Pointer>, T>; }
  void Emit(Cast<u32, i32>& op) final { result = std::is_same_v<Cast<u32, i32>, T>; }
  void Emit(Cast<u32, u32>& op) final { result = std::is_same_v<Cast<u32, u32>, T>; }
  void Emit(Cast<u32, i64>& op) final { result = std::is_same_v<Cast<u32, i64>, T>; }
  void Emit(Cast<u32, u64>& op) final { result = std::is_same_v<Cast<u32, u64>, T>; }
  void Emit(Cast<u32, float>& op) final { result = std::is_same_v<Cast<u32, float>, T>; }
  void Emit(Cast<u32, double>& op) final { result = std::is_same_v<Cast<u32, double>, T>; }
  void Emit(Cast<u32, Pointer>& op) final { result = std::is_same_v<Cast<u32, Pointer>, T>; }
  void Emit(Cast<u64, i32>& op) final { result = std::is_same_v<Cast<u64, i32>, T>; }
  void Emit(Cast<u64, u32>& op) final { result = std::is_same_v<Cast<u64, u32>, T>; }
  void Emit(Cast<u64, i64>& op) final { result = std::is_same_v<Cast<u64, i64>, T>; }
  void Emit(Cast<u64, u64>& op) final { result = std::is_same_v<Cast<u64, u64>, T>; }
  void Emit(Cast<u64, float>& op) final { result = std::is_same_v<Cast<u64, float>, T>; }
  void Emit(Cast<u64, double>& op) final { result = std::is_same_v<Cast<u64, double>, T>; }
  void Emit(Cast<u64, Pointer>& op) final { result = std::is_same_v<Cast<u64, Pointer>, T>; }
  void Emit(Cast<float, i32>& op) final { result = std::is_same_v<Cast<float, i32>, T>; }
  void Emit(Cast<float, u32>& op) final { result = std::is_same_v<Cast<float, u32>, T>; }
  void Emit(Cast<float, i64>& op) final { result = std::is_same_v<Cast<float, i64>, T>; }
  void Emit(Cast<float, u64>& op) final { result = std::is_same_v<Cast<float, u64>, T>; }
  void Emit(Cast<float, float>& op) final { result = std::is_same_v<Cast<float, float>, T>; }
  void Emit(Cast<float, double>& op) final { result = std::is_same_v<Cast<float, double>, T>; }
  void Emit(Cast<float, Pointer>& op) final { result = std::is_same_v<Cast<float, Pointer>, T>; }
  void Emit(Cast<double, i32>& op) final { result = std::is_same_v<Cast<double, i32>, T>; }
  void Emit(Cast<double, u32>& op) final { result = std::is_same_v<Cast<double, u32>, T>; }
  void Emit(Cast<double, i64>& op) final { result = std::is_same_v<Cast<double, i64>, T>; }
  void Emit(Cast<double, u64>& op) final { result = std::is_same_v<Cast<double, u64>, T>; }
  void Emit(Cast<double, float>& op) final { result = std::is_same_v<Cast<double, float>, T>; }
  void Emit(Cast<double, double>& op) final { result = std::is_same_v<Cast<double, double>, T>; }
  void Emit(Cast<double, Pointer>& op) final { result = std::is_same_v<Cast<double, Pointer>, T>; }
  void Emit(Cast<Pointer, i32>& op) final { result = std::is_same_v<Cast<Pointer, i32>, T>; }
  void Emit(Cast<Pointer, u32>& op) final { result = std::is_same_v<Cast<Pointer, u32>, T>; }
  void Emit(Cast<Pointer, i64>& op) final { result = std::is_same_v<Cast<Pointer, i64>, T>; }
  void Emit(Cast<Pointer, u64>& op) final { result = std::is_same_v<Cast<Pointer, u64>, T>; }
  void Emit(Cast<Pointer, float>& op) final { result = std::is_same_v<Cast<Pointer, float>, T>; }
  void Emit(Cast<Pointer, double>& op) final { result = std::is_same_v<Cast<Pointer, double>, T>; }
  void Emit(Cast<Pointer, Pointer>& op) final { result = std::is_same_v<Cast<Pointer, Pointer>, T>; }
};

template<typename T>
bool IsType(pDirective& directive) {
  IRCompare<T> visitor{};
  directive->Emit(visitor);
  return visitor.result;
}

}