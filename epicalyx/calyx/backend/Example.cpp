#include "Example.h"


namespace epi::calyx {

void Example::EmitProgram(std::vector<calyx::pDirective>& program) {

}

void Example::Emit(AllocateCVar& op) {

}

void Example::Emit(DeallocateCVar& op) {

}

void Example::Emit(LoadCVarAddr& op) {

}

template<typename To, typename From>
void Example::EmitCast(Cast<To, From>& op) {

}

template<typename T>
void Example::EmitLoadCVar(LoadCVar<T>& op) {

}

template<typename T>
void Example::EmitStoreCVar(StoreCVar<T>& op) {

}

template<typename T>
void Example::EmitReturn(Return<T>& op) {

}

template<typename T>
void Example::EmitImm(Imm<T>& op) {

}

template<typename T>
void Example::EmitUnop(Unop<T>& op) {

}

template<typename T>
void Example::EmitBinop(Binop<T>& op) {

}

template<typename T>
void Example::EmitAddToPointer(AddToPointer<T>& op) {

}


void Example::Emit(Binop<i32>& op) { EmitBinop(op); }
void Example::Emit(Binop<u32>& op) { EmitBinop(op); }
void Example::Emit(Binop<i64>& op) { EmitBinop(op); }
void Example::Emit(Binop<u64>& op) { EmitBinop(op); }
void Example::Emit(Binop<float>& op) { EmitBinop(op); }
void Example::Emit(Binop<double>& op) { EmitBinop(op); }
void Example::Emit(AddToPointer<i32>& op) { EmitAddToPointer(op); }
void Example::Emit(AddToPointer<u32>& op) { EmitAddToPointer(op); }
void Example::Emit(AddToPointer<i64>& op) { EmitAddToPointer(op); }
void Example::Emit(AddToPointer<u64>& op) { EmitAddToPointer(op); }
void Example::Emit(Unop<i32>& op) { EmitUnop(op); }
void Example::Emit(Unop<u32>& op) { EmitUnop(op); }
void Example::Emit(Unop<i64>& op) { EmitUnop(op); }
void Example::Emit(Unop<u64>& op) { EmitUnop(op); }
void Example::Emit(Unop<float>& op) { EmitUnop(op); }
void Example::Emit(Unop<double>& op) { EmitUnop(op); }
void Example::Emit(Imm<i32>& op) { EmitImm(op); }
void Example::Emit(Imm<u32>& op) { EmitImm(op); }
void Example::Emit(Imm<i64>& op) { EmitImm(op); }
void Example::Emit(Imm<u64>& op) { EmitImm(op); }
void Example::Emit(Imm<float>& op) { EmitImm(op); }
void Example::Emit(Imm<double>& op) { EmitImm(op); }
void Example::Emit(LoadCVar<i8>& op) { EmitLoadCVar(op); }
void Example::Emit(LoadCVar<u8>& op) { EmitLoadCVar(op); }
void Example::Emit(LoadCVar<i16>& op) { EmitLoadCVar(op); }
void Example::Emit(LoadCVar<u16>& op) { EmitLoadCVar(op); }
void Example::Emit(LoadCVar<i32>& op) { EmitLoadCVar(op); }
void Example::Emit(LoadCVar<u32>& op) { EmitLoadCVar(op); }
void Example::Emit(LoadCVar<i64>& op) { EmitLoadCVar(op); }
void Example::Emit(LoadCVar<u64>& op) { EmitLoadCVar(op); }
void Example::Emit(LoadCVar<float>& op) { EmitLoadCVar(op); }
void Example::Emit(LoadCVar<double>& op) { EmitLoadCVar(op); }
void Example::Emit(LoadCVar<Struct>& op) { EmitLoadCVar(op); }
void Example::Emit(LoadCVar<Pointer>& op) { EmitLoadCVar(op); }
void Example::Emit(StoreCVar<i8>& op) { EmitStoreCVar(op); }
void Example::Emit(StoreCVar<u8>& op) { EmitStoreCVar(op); }
void Example::Emit(StoreCVar<i16>& op) { EmitStoreCVar(op); }
void Example::Emit(StoreCVar<u16>& op) { EmitStoreCVar(op); }
void Example::Emit(StoreCVar<i32>& op) { EmitStoreCVar(op); }
void Example::Emit(StoreCVar<u32>& op) { EmitStoreCVar(op); }
void Example::Emit(StoreCVar<i64>& op) { EmitStoreCVar(op); }
void Example::Emit(StoreCVar<u64>& op) { EmitStoreCVar(op); }
void Example::Emit(StoreCVar<float>& op) { EmitStoreCVar(op); }
void Example::Emit(StoreCVar<double>& op) { EmitStoreCVar(op); }
void Example::Emit(StoreCVar<Struct>& op) { EmitStoreCVar(op); }
void Example::Emit(StoreCVar<Pointer>& op) { EmitStoreCVar(op); }
void Example::Emit(Return<i32>& op) { EmitReturn(op); }
void Example::Emit(Return<u32>& op) { EmitReturn(op); }
void Example::Emit(Return<i64>& op) { EmitReturn(op); }
void Example::Emit(Return<u64>& op) { EmitReturn(op); }
void Example::Emit(Return<float>& op) { EmitReturn(op); }
void Example::Emit(Return<double>& op) { EmitReturn(op); }
void Example::Emit(Return<Struct>& op) { EmitReturn(op); }
void Example::Emit(Return<Pointer>& op) { EmitReturn(op); }
void Example::Emit(Cast<i8, i32>& op) { EmitCast(op); }
void Example::Emit(Cast<i8, u32>& op) { EmitCast(op); }
void Example::Emit(Cast<i8, i64>& op) { EmitCast(op); }
void Example::Emit(Cast<i8, u64>& op) { EmitCast(op); }
void Example::Emit(Cast<i8, float>& op) { EmitCast(op); }
void Example::Emit(Cast<i8, double>& op) { EmitCast(op); }
void Example::Emit(Cast<i8, Pointer>& op) { EmitCast(op); }
void Example::Emit(Cast<i16, i32>& op) { EmitCast(op); }
void Example::Emit(Cast<i16, u32>& op) { EmitCast(op); }
void Example::Emit(Cast<i16, i64>& op) { EmitCast(op); }
void Example::Emit(Cast<i16, u64>& op) { EmitCast(op); }
void Example::Emit(Cast<i16, float>& op) { EmitCast(op); }
void Example::Emit(Cast<i16, double>& op) { EmitCast(op); }
void Example::Emit(Cast<i16, Pointer>& op) { EmitCast(op); }
void Example::Emit(Cast<i32, i32>& op) { EmitCast(op); }
void Example::Emit(Cast<i32, u32>& op) { EmitCast(op); }
void Example::Emit(Cast<i32, i64>& op) { EmitCast(op); }
void Example::Emit(Cast<i32, u64>& op) { EmitCast(op); }
void Example::Emit(Cast<i32, float>& op) { EmitCast(op); }
void Example::Emit(Cast<i32, double>& op) { EmitCast(op); }
void Example::Emit(Cast<i32, Pointer>& op) { EmitCast(op); }
void Example::Emit(Cast<i64, i32>& op) { EmitCast(op); }
void Example::Emit(Cast<i64, u32>& op) { EmitCast(op); }
void Example::Emit(Cast<i64, i64>& op) { EmitCast(op); }
void Example::Emit(Cast<i64, u64>& op) { EmitCast(op); }
void Example::Emit(Cast<i64, float>& op) { EmitCast(op); }
void Example::Emit(Cast<i64, double>& op) { EmitCast(op); }
void Example::Emit(Cast<i64, Pointer>& op) { EmitCast(op); }
void Example::Emit(Cast<u8, i32>& op) { EmitCast(op); }
void Example::Emit(Cast<u8, u32>& op) { EmitCast(op); }
void Example::Emit(Cast<u8, i64>& op) { EmitCast(op); }
void Example::Emit(Cast<u8, u64>& op) { EmitCast(op); }
void Example::Emit(Cast<u8, float>& op) { EmitCast(op); }
void Example::Emit(Cast<u8, double>& op) { EmitCast(op); }
void Example::Emit(Cast<u8, Pointer>& op) { EmitCast(op); }
void Example::Emit(Cast<u16, i32>& op) { EmitCast(op); }
void Example::Emit(Cast<u16, u32>& op) { EmitCast(op); }
void Example::Emit(Cast<u16, i64>& op) { EmitCast(op); }
void Example::Emit(Cast<u16, u64>& op) { EmitCast(op); }
void Example::Emit(Cast<u16, float>& op) { EmitCast(op); }
void Example::Emit(Cast<u16, double>& op) { EmitCast(op); }
void Example::Emit(Cast<u16, Pointer>& op) { EmitCast(op); }
void Example::Emit(Cast<u32, i32>& op) { EmitCast(op); }
void Example::Emit(Cast<u32, u32>& op) { EmitCast(op); }
void Example::Emit(Cast<u32, i64>& op) { EmitCast(op); }
void Example::Emit(Cast<u32, u64>& op) { EmitCast(op); }
void Example::Emit(Cast<u32, float>& op) { EmitCast(op); }
void Example::Emit(Cast<u32, double>& op) { EmitCast(op); }
void Example::Emit(Cast<u32, Pointer>& op) { EmitCast(op); }
void Example::Emit(Cast<u64, i32>& op) { EmitCast(op); }
void Example::Emit(Cast<u64, u32>& op) { EmitCast(op); }
void Example::Emit(Cast<u64, i64>& op) { EmitCast(op); }
void Example::Emit(Cast<u64, u64>& op) { EmitCast(op); }
void Example::Emit(Cast<u64, float>& op) { EmitCast(op); }
void Example::Emit(Cast<u64, double>& op) { EmitCast(op); }
void Example::Emit(Cast<u64, Pointer>& op) { EmitCast(op); }
void Example::Emit(Cast<float, i32>& op) { EmitCast(op); }
void Example::Emit(Cast<float, u32>& op) { EmitCast(op); }
void Example::Emit(Cast<float, i64>& op) { EmitCast(op); }
void Example::Emit(Cast<float, u64>& op) { EmitCast(op); }
void Example::Emit(Cast<float, float>& op) { EmitCast(op); }
void Example::Emit(Cast<float, double>& op) { EmitCast(op); }
void Example::Emit(Cast<float, Pointer>& op) { EmitCast(op); }
void Example::Emit(Cast<double, i32>& op) { EmitCast(op); }
void Example::Emit(Cast<double, u32>& op) { EmitCast(op); }
void Example::Emit(Cast<double, i64>& op) { EmitCast(op); }
void Example::Emit(Cast<double, u64>& op) { EmitCast(op); }
void Example::Emit(Cast<double, float>& op) { EmitCast(op); }
void Example::Emit(Cast<double, double>& op) { EmitCast(op); }
void Example::Emit(Cast<double, Pointer>& op) { EmitCast(op); }
void Example::Emit(Cast<Pointer, i32>& op) { EmitCast(op); }
void Example::Emit(Cast<Pointer, u32>& op) { EmitCast(op); }
void Example::Emit(Cast<Pointer, i64>& op) { EmitCast(op); }
void Example::Emit(Cast<Pointer, u64>& op) { EmitCast(op); }
void Example::Emit(Cast<Pointer, float>& op) { EmitCast(op); }
void Example::Emit(Cast<Pointer, double>& op) { EmitCast(op); }
void Example::Emit(Cast<Pointer, Pointer>& op) { EmitCast(op); }

}