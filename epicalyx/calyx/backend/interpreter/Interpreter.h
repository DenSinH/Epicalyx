#pragma once

#include "calyx/backend/Backend.h"

#include <variant>


namespace epi::calyx {

struct Interpreter : Backend {

  std::vector<u8> stack{};

  // id 0 is special
  std::vector<u64> c_vars{0};
  std::vector<std::variant<i32, u32, i64, u64, float, double>> vars{{}};

  void EmitProgram(std::vector<calyx::pDirective>& program) final;

  void Emit(AllocateCVar& op) final;
  void Emit(DeallocateCVar& op) final;
  void Emit(LoadCVarAddr& op) final;

  template<typename To, typename From>
  void EmitCast(Cast<To, From>& op);
  template<typename T>
  void EmitLoadCVar(LoadCVar<T>& op);
  template<typename T>
  void EmitStoreCVar(StoreCVar<T>& op);
  template<typename T>
  void EmitReturn(Return<T>& op);
  template<typename T>
  void EmitImm(Imm<T>& op);
  template<typename T>
  void EmitUnop(Unop<T>& op);
  template<typename T>
  void EmitBinop(Binop<T>& op);
  template<typename T>
  void EmitAddToPointer(AddToPointer<T>& op);

  void Emit(Binop<i32>& op) final;
  void Emit(Binop<u32>& op) final;
  void Emit(Binop<i64>& op) final;
  void Emit(Binop<u64>& op) final;
  void Emit(Binop<float>& op) final;
  void Emit(Binop<double>& op) final;
  void Emit(AddToPointer<i32>& op) final;
  void Emit(AddToPointer<u32>& op) final;
  void Emit(AddToPointer<i64>& op) final;
  void Emit(AddToPointer<u64>& op) final;
  void Emit(Unop<i32>& op) final;
  void Emit(Unop<u32>& op) final;
  void Emit(Unop<i64>& op) final;
  void Emit(Unop<u64>& op) final;
  void Emit(Unop<float>& op) final;
  void Emit(Unop<double>& op) final;
  void Emit(Imm<i32>& op) final;
  void Emit(Imm<u32>& op) final;
  void Emit(Imm<i64>& op) final;
  void Emit(Imm<u64>& op) final;
  void Emit(Imm<float>& op) final;
  void Emit(Imm<double>& op) final;
  void Emit(LoadCVar<i8>& op) final;
  void Emit(LoadCVar<u8>& op) final;
  void Emit(LoadCVar<i16>& op) final;
  void Emit(LoadCVar<u16>& op) final;
  void Emit(LoadCVar<i32>& op) final;
  void Emit(LoadCVar<u32>& op) final;
  void Emit(LoadCVar<i64>& op) final;
  void Emit(LoadCVar<u64>& op) final;
  void Emit(LoadCVar<float>& op) final;
  void Emit(LoadCVar<double>& op) final;
  void Emit(LoadCVar<Struct>& op) final;
  void Emit(LoadCVar<Pointer>& op) final;
  void Emit(StoreCVar<i8>& op) final;
  void Emit(StoreCVar<u8>& op) final;
  void Emit(StoreCVar<i16>& op) final;
  void Emit(StoreCVar<u16>& op) final;
  void Emit(StoreCVar<i32>& op) final;
  void Emit(StoreCVar<u32>& op) final;
  void Emit(StoreCVar<i64>& op) final;
  void Emit(StoreCVar<u64>& op) final;
  void Emit(StoreCVar<float>& op) final;
  void Emit(StoreCVar<double>& op) final;
  void Emit(StoreCVar<Struct>& op) final;
  void Emit(StoreCVar<Pointer>& op) final;
  void Emit(Return<i32>& op) final;
  void Emit(Return<u32>& op) final;
  void Emit(Return<i64>& op) final;
  void Emit(Return<u64>& op) final;
  void Emit(Return<float>& op) final;
  void Emit(Return<double>& op) final;
  void Emit(Return<Struct>& op) final;
  void Emit(Return<Pointer>& op) final;
  void Emit(Cast<i8, i32>& op) final;
  void Emit(Cast<i8, u32>& op) final;
  void Emit(Cast<i8, i64>& op) final;
  void Emit(Cast<i8, u64>& op) final;
  void Emit(Cast<i8, float>& op) final;
  void Emit(Cast<i8, double>& op) final;
  void Emit(Cast<i8, Pointer>& op) final;
  void Emit(Cast<i16, i32>& op) final;
  void Emit(Cast<i16, u32>& op) final;
  void Emit(Cast<i16, i64>& op) final;
  void Emit(Cast<i16, u64>& op) final;
  void Emit(Cast<i16, float>& op) final;
  void Emit(Cast<i16, double>& op) final;
  void Emit(Cast<i16, Pointer>& op) final;
  void Emit(Cast<i32, i32>& op) final;
  void Emit(Cast<i32, u32>& op) final;
  void Emit(Cast<i32, i64>& op) final;
  void Emit(Cast<i32, u64>& op) final;
  void Emit(Cast<i32, float>& op) final;
  void Emit(Cast<i32, double>& op) final;
  void Emit(Cast<i32, Pointer>& op) final;
  void Emit(Cast<i64, i32>& op) final;
  void Emit(Cast<i64, u32>& op) final;
  void Emit(Cast<i64, i64>& op) final;
  void Emit(Cast<i64, u64>& op) final;
  void Emit(Cast<i64, float>& op) final;
  void Emit(Cast<i64, double>& op) final;
  void Emit(Cast<i64, Pointer>& op) final;
  void Emit(Cast<u8, i32>& op) final;
  void Emit(Cast<u8, u32>& op) final;
  void Emit(Cast<u8, i64>& op) final;
  void Emit(Cast<u8, u64>& op) final;
  void Emit(Cast<u8, float>& op) final;
  void Emit(Cast<u8, double>& op) final;
  void Emit(Cast<u8, Pointer>& op) final;
  void Emit(Cast<u16, i32>& op) final;
  void Emit(Cast<u16, u32>& op) final;
  void Emit(Cast<u16, i64>& op) final;
  void Emit(Cast<u16, u64>& op) final;
  void Emit(Cast<u16, float>& op) final;
  void Emit(Cast<u16, double>& op) final;
  void Emit(Cast<u16, Pointer>& op) final;
  void Emit(Cast<u32, i32>& op) final;
  void Emit(Cast<u32, u32>& op) final;
  void Emit(Cast<u32, i64>& op) final;
  void Emit(Cast<u32, u64>& op) final;
  void Emit(Cast<u32, float>& op) final;
  void Emit(Cast<u32, double>& op) final;
  void Emit(Cast<u32, Pointer>& op) final;
  void Emit(Cast<u64, i32>& op) final;
  void Emit(Cast<u64, u32>& op) final;
  void Emit(Cast<u64, i64>& op) final;
  void Emit(Cast<u64, u64>& op) final;
  void Emit(Cast<u64, float>& op) final;
  void Emit(Cast<u64, double>& op) final;
  void Emit(Cast<u64, Pointer>& op) final;
  void Emit(Cast<float, i32>& op) final;
  void Emit(Cast<float, u32>& op) final;
  void Emit(Cast<float, i64>& op) final;
  void Emit(Cast<float, u64>& op) final;
  void Emit(Cast<float, float>& op) final;
  void Emit(Cast<float, double>& op) final;
  void Emit(Cast<float, Pointer>& op) final;
  void Emit(Cast<double, i32>& op) final;
  void Emit(Cast<double, u32>& op) final;
  void Emit(Cast<double, i64>& op) final;
  void Emit(Cast<double, u64>& op) final;
  void Emit(Cast<double, float>& op) final;
  void Emit(Cast<double, double>& op) final;
  void Emit(Cast<double, Pointer>& op) final;
  void Emit(Cast<Pointer, i32>& op) final;
  void Emit(Cast<Pointer, u32>& op) final;
  void Emit(Cast<Pointer, i64>& op) final;
  void Emit(Cast<Pointer, u64>& op) final;
  void Emit(Cast<Pointer, float>& op) final;
  void Emit(Cast<Pointer, double>& op) final;
  void Emit(Cast<Pointer, Pointer>& op) final;
};

}