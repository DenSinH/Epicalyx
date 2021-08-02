#pragma once


#include "Backend.h"

namespace epi::calyx {

struct Interpreter : Backend {

  void EmitProgram(std::vector<calyx::pDirective>& program) final;

  void Emit(AllocateCVar& op) final;
  void Emit(DeallocateCVar& op) final;
  void Emit(LoadCVarAddr& op) final;
  void Emit(Return& op) final;

  template<typename To, typename From>
  void EmitCast(Cast<To, From>& op);
  template<typename T>
  void EmitStoreCVar(StoreCVar<T>& op);
  template<typename T>
  void EmitLoadCVar(LoadCVar<T>& op);
  template<typename T>
  void EmitImm(Imm<T>& op);
  template<typename T>
  void EmitUnop(Unop<T>& op);
  template<typename T>
  void EmitBinop(Binop<T>& op);
  template<typename T>
  void EmitAddToPointer(AddToPointer<T>& op);

  void Emit(Binop<i32>& op) final { EmitBinop(op); }
  void Emit(Binop<u32>& op) final { EmitBinop(op); }
  void Emit(Binop<i64>& op) final { EmitBinop(op); }
  void Emit(Binop<u64>& op) final { EmitBinop(op); }
  void Emit(Binop<float>& op) final { EmitBinop(op); }
  void Emit(Binop<double>& op) final { EmitBinop(op); }
  void Emit(Binop<Pointer>& op) final { EmitBinop(op); }
  void Emit(AddToPointer<i32>& op) final { EmitAddToPointer(op); }
  void Emit(AddToPointer<u32>& op) final { EmitAddToPointer(op); }
  void Emit(AddToPointer<i64>& op) final { EmitAddToPointer(op); }
  void Emit(AddToPointer<u64>& op) final { EmitAddToPointer(op); }
  void Emit(Unop<i32>& op) final { EmitUnop(op); }
  void Emit(Unop<u32>& op) final { EmitUnop(op); }
  void Emit(Unop<i64>& op) final { EmitUnop(op); }
  void Emit(Unop<u64>& op) final { EmitUnop(op); }
  void Emit(Unop<float>& op) final { EmitUnop(op); }
  void Emit(Unop<double>& op) final { EmitUnop(op); }
  void Emit(Unop<Pointer>& op) final { EmitUnop(op); }
  void Emit(Imm<i32>& op) final { EmitImm(op); }
  void Emit(Imm<u32>& op) final { EmitImm(op); }
  void Emit(Imm<i64>& op) final { EmitImm(op); }
  void Emit(Imm<u64>& op) final { EmitImm(op); }
  void Emit(Imm<float>& op) final { EmitImm(op); }
  void Emit(Imm<double>& op) final { EmitImm(op); }
  void Emit(LoadCVar<i8>& op) final { EmitLoadCVar(op); }
  void Emit(LoadCVar<u8>& op) final { EmitLoadCVar(op); }
  void Emit(LoadCVar<i16>& op) final { EmitLoadCVar(op); }
  void Emit(LoadCVar<u16>& op) final { EmitLoadCVar(op); }
  void Emit(LoadCVar<i32>& op) final { EmitLoadCVar(op); }
  void Emit(LoadCVar<u32>& op) final { EmitLoadCVar(op); }
  void Emit(LoadCVar<i64>& op) final { EmitLoadCVar(op); }
  void Emit(LoadCVar<u64>& op) final { EmitLoadCVar(op); }
  void Emit(LoadCVar<float>& op) final { EmitLoadCVar(op); }
  void Emit(LoadCVar<double>& op) final { EmitLoadCVar(op); }
  void Emit(LoadCVar<Struct>& op) final { EmitLoadCVar(op); }
  void Emit(LoadCVar<Pointer>& op) final { EmitLoadCVar(op); }
  void Emit(StoreCVar<i8>& op) final { EmitStoreCVar(op); }
  void Emit(StoreCVar<u8>& op) final { EmitStoreCVar(op); }
  void Emit(StoreCVar<i16>& op) final { EmitStoreCVar(op); }
  void Emit(StoreCVar<u16>& op) final { EmitStoreCVar(op); }
  void Emit(StoreCVar<i32>& op) final { EmitStoreCVar(op); }
  void Emit(StoreCVar<u32>& op) final { EmitStoreCVar(op); }
  void Emit(StoreCVar<i64>& op) final { EmitStoreCVar(op); }
  void Emit(StoreCVar<u64>& op) final { EmitStoreCVar(op); }
  void Emit(StoreCVar<float>& op) final { EmitStoreCVar(op); }
  void Emit(StoreCVar<double>& op) final { EmitStoreCVar(op); }
  void Emit(StoreCVar<Struct>& op) final { EmitStoreCVar(op); }
  void Emit(StoreCVar<Pointer>& op) final { EmitStoreCVar(op); }
  void Emit(Cast<i8, i32>& op) final { EmitCast(op); }
  void Emit(Cast<i8, u32>& op) final { EmitCast(op); }
  void Emit(Cast<i8, i64>& op) final { EmitCast(op); }
  void Emit(Cast<i8, u64>& op) final { EmitCast(op); }
  void Emit(Cast<i8, float>& op) final { EmitCast(op); }
  void Emit(Cast<i8, double>& op) final { EmitCast(op); }
  void Emit(Cast<i8, Pointer>& op) final { EmitCast(op); }
  void Emit(Cast<i16, i32>& op) final { EmitCast(op); }
  void Emit(Cast<i16, u32>& op) final { EmitCast(op); }
  void Emit(Cast<i16, i64>& op) final { EmitCast(op); }
  void Emit(Cast<i16, u64>& op) final { EmitCast(op); }
  void Emit(Cast<i16, float>& op) final { EmitCast(op); }
  void Emit(Cast<i16, double>& op) final { EmitCast(op); }
  void Emit(Cast<i16, Pointer>& op) final { EmitCast(op); }
  void Emit(Cast<i32, i32>& op) final { EmitCast(op); }
  void Emit(Cast<i32, u32>& op) final { EmitCast(op); }
  void Emit(Cast<i32, i64>& op) final { EmitCast(op); }
  void Emit(Cast<i32, u64>& op) final { EmitCast(op); }
  void Emit(Cast<i32, float>& op) final { EmitCast(op); }
  void Emit(Cast<i32, double>& op) final { EmitCast(op); }
  void Emit(Cast<i32, Pointer>& op) final { EmitCast(op); }
  void Emit(Cast<i64, i32>& op) final { EmitCast(op); }
  void Emit(Cast<i64, u32>& op) final { EmitCast(op); }
  void Emit(Cast<i64, i64>& op) final { EmitCast(op); }
  void Emit(Cast<i64, u64>& op) final { EmitCast(op); }
  void Emit(Cast<i64, float>& op) final { EmitCast(op); }
  void Emit(Cast<i64, double>& op) final { EmitCast(op); }
  void Emit(Cast<i64, Pointer>& op) final { EmitCast(op); }
  void Emit(Cast<u8, i32>& op) final { EmitCast(op); }
  void Emit(Cast<u8, u32>& op) final { EmitCast(op); }
  void Emit(Cast<u8, i64>& op) final { EmitCast(op); }
  void Emit(Cast<u8, u64>& op) final { EmitCast(op); }
  void Emit(Cast<u8, float>& op) final { EmitCast(op); }
  void Emit(Cast<u8, double>& op) final { EmitCast(op); }
  void Emit(Cast<u8, Pointer>& op) final { EmitCast(op); }
  void Emit(Cast<u16, i32>& op) final { EmitCast(op); }
  void Emit(Cast<u16, u32>& op) final { EmitCast(op); }
  void Emit(Cast<u16, i64>& op) final { EmitCast(op); }
  void Emit(Cast<u16, u64>& op) final { EmitCast(op); }
  void Emit(Cast<u16, float>& op) final { EmitCast(op); }
  void Emit(Cast<u16, double>& op) final { EmitCast(op); }
  void Emit(Cast<u16, Pointer>& op) final { EmitCast(op); }
  void Emit(Cast<u32, i32>& op) final { EmitCast(op); }
  void Emit(Cast<u32, u32>& op) final { EmitCast(op); }
  void Emit(Cast<u32, i64>& op) final { EmitCast(op); }
  void Emit(Cast<u32, u64>& op) final { EmitCast(op); }
  void Emit(Cast<u32, float>& op) final { EmitCast(op); }
  void Emit(Cast<u32, double>& op) final { EmitCast(op); }
  void Emit(Cast<u32, Pointer>& op) final { EmitCast(op); }
  void Emit(Cast<u64, i32>& op) final { EmitCast(op); }
  void Emit(Cast<u64, u32>& op) final { EmitCast(op); }
  void Emit(Cast<u64, i64>& op) final { EmitCast(op); }
  void Emit(Cast<u64, u64>& op) final { EmitCast(op); }
  void Emit(Cast<u64, float>& op) final { EmitCast(op); }
  void Emit(Cast<u64, double>& op) final { EmitCast(op); }
  void Emit(Cast<u64, Pointer>& op) final { EmitCast(op); }
  void Emit(Cast<float, i32>& op) final { EmitCast(op); }
  void Emit(Cast<float, u32>& op) final { EmitCast(op); }
  void Emit(Cast<float, i64>& op) final { EmitCast(op); }
  void Emit(Cast<float, u64>& op) final { EmitCast(op); }
  void Emit(Cast<float, float>& op) final { EmitCast(op); }
  void Emit(Cast<float, double>& op) final { EmitCast(op); }
  void Emit(Cast<float, Pointer>& op) final { EmitCast(op); }
  void Emit(Cast<double, i32>& op) final { EmitCast(op); }
  void Emit(Cast<double, u32>& op) final { EmitCast(op); }
  void Emit(Cast<double, i64>& op) final { EmitCast(op); }
  void Emit(Cast<double, u64>& op) final { EmitCast(op); }
  void Emit(Cast<double, float>& op) final { EmitCast(op); }
  void Emit(Cast<double, double>& op) final { EmitCast(op); }
  void Emit(Cast<double, Pointer>& op) final { EmitCast(op); }
  void Emit(Cast<Pointer, i32>& op) final { EmitCast(op); }
  void Emit(Cast<Pointer, u32>& op) final { EmitCast(op); }
  void Emit(Cast<Pointer, i64>& op) final { EmitCast(op); }
  void Emit(Cast<Pointer, u64>& op) final { EmitCast(op); }
  void Emit(Cast<Pointer, float>& op) final { EmitCast(op); }
  void Emit(Cast<Pointer, double>& op) final { EmitCast(op); }
  void Emit(Cast<Pointer, Pointer>& op) final { EmitCast(op); }
};

}