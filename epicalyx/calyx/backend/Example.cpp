#include "Example.h"


namespace epi::calyx {

void Example::EmitProgram(Program& program) {

}

void Example::Emit(AllocateLocal& op) {

}

void Example::Emit(DeallocateLocal& op) {

}

void Example::Emit(LoadLocalAddr& op) {

}

template<typename To, typename From>
void Example::EmitCast(Cast<To, From>& op) {

}

template<typename T>
void Example::EmitLoadLocal(LoadLocal<T>& op) {

}

template<typename T>
void Example::EmitStoreLocal(StoreLocal<T>& op) {

}

template<typename T>
void Example::EmitLoadFromPointer(LoadFromPointer<T>& op) {
  
}

template<typename T>
void Example::EmitStoreToPointer(StoreToPointer<T>& op) {
  
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
void Example::EmitBinopImm(BinopImm<T>& op) {

}

template<typename T>
void Example::EmitShift(Shift<T>& op) {

}

template<typename T>
void Example::EmitShiftImm(ShiftImm<T>& op) {

}

template<typename T>
void Example::EmitCompare(Compare<T>& op) {

}

template<typename T>
void Example::EmitCompareImm(CompareImm<T>& op) {

}

void Example::Emit(UnconditionalBranch& op) {

}

template<typename T>
void Example::EmitBranchCompare(BranchCompare<T>& op) {

}

template<typename T>
void Example::EmitBranchCompareImm(BranchCompareImm<T>& op) {

}

void Example::Emit(Select& op) {

}

template<typename T>
void Example::EmitAddToPointer(AddToPointer<T>& op) {

}

void Example::Emit(AddToPointerImm& op) {

}

void Example::Emit(Binop<i32>& op) { EmitBinop(op); }
void Example::Emit(Binop<u32>& op) { EmitBinop(op); }
void Example::Emit(Binop<i64>& op) { EmitBinop(op); }
void Example::Emit(Binop<u64>& op) { EmitBinop(op); }
void Example::Emit(Binop<float>& op) { EmitBinop(op); }
void Example::Emit(Binop<double>& op) { EmitBinop(op); }
void Example::Emit(BinopImm<i32>& op) { EmitBinopImm(op); }
void Example::Emit(BinopImm<u32>& op) { EmitBinopImm(op); }
void Example::Emit(BinopImm<i64>& op) { EmitBinopImm(op); }
void Example::Emit(BinopImm<u64>& op) { EmitBinopImm(op); }
void Example::Emit(BinopImm<float>& op) { EmitBinopImm(op); }
void Example::Emit(BinopImm<double>& op) { EmitBinopImm(op); }
void Example::Emit(Shift<i32>& op) { EmitShift(op); }
void Example::Emit(Shift<u32>& op) { EmitShift(op); }
void Example::Emit(Shift<i64>& op) { EmitShift(op); }
void Example::Emit(Shift<u64>& op) { EmitShift(op); }
void Example::Emit(ShiftImm<i32>& op) { EmitShiftImm(op); }
void Example::Emit(ShiftImm<u32>& op) { EmitShiftImm(op); }
void Example::Emit(ShiftImm<i64>& op) { EmitShiftImm(op); }
void Example::Emit(ShiftImm<u64>& op) { EmitShiftImm(op); }
void Example::Emit(Compare<i32>& op) { EmitCompare(op); }
void Example::Emit(Compare<u32>& op) { EmitCompare(op); }
void Example::Emit(Compare<i64>& op) { EmitCompare(op); }
void Example::Emit(Compare<u64>& op) { EmitCompare(op); }
void Example::Emit(Compare<float>& op) { EmitCompare(op); }
void Example::Emit(Compare<double>& op) { EmitCompare(op); }
void Example::Emit(Compare<Pointer>& op) { EmitCompare(op); }
void Example::Emit(CompareImm<i32>& op) { EmitCompareImm(op); }
void Example::Emit(CompareImm<u32>& op) { EmitCompareImm(op); }
void Example::Emit(CompareImm<i64>& op) { EmitCompareImm(op); }
void Example::Emit(CompareImm<u64>& op) { EmitCompareImm(op); }
void Example::Emit(CompareImm<float>& op) { EmitCompareImm(op); }
void Example::Emit(CompareImm<double>& op) { EmitCompareImm(op); }
void Example::Emit(CompareImm<Pointer>& op) { EmitCompareImm(op); }
void Example::Emit(BranchCompare<i32>& op) { EmitBranchCompare(op); }
void Example::Emit(BranchCompare<u32>& op) { EmitBranchCompare(op); }
void Example::Emit(BranchCompare<i64>& op) { EmitBranchCompare(op); }
void Example::Emit(BranchCompare<u64>& op) { EmitBranchCompare(op); }
void Example::Emit(BranchCompare<float>& op) { EmitBranchCompare(op); }
void Example::Emit(BranchCompare<double>& op) { EmitBranchCompare(op); }
void Example::Emit(BranchCompare<Pointer>& op) { EmitBranchCompare(op); }
void Example::Emit(BranchCompareImm<i32>& op) { EmitBranchCompareImm(op); }
void Example::Emit(BranchCompareImm<u32>& op) { EmitBranchCompareImm(op); }
void Example::Emit(BranchCompareImm<i64>& op) { EmitBranchCompareImm(op); }
void Example::Emit(BranchCompareImm<u64>& op) { EmitBranchCompareImm(op); }
void Example::Emit(BranchCompareImm<float>& op) { EmitBranchCompareImm(op); }
void Example::Emit(BranchCompareImm<double>& op) { EmitBranchCompareImm(op); }
void Example::Emit(BranchCompareImm<Pointer>& op) { EmitBranchCompareImm(op); }
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
void Example::Emit(LoadLocal<i8>& op) { EmitLoadLocal(op); }
void Example::Emit(LoadLocal<u8>& op) { EmitLoadLocal(op); }
void Example::Emit(LoadLocal<i16>& op) { EmitLoadLocal(op); }
void Example::Emit(LoadLocal<u16>& op) { EmitLoadLocal(op); }
void Example::Emit(LoadLocal<i32>& op) { EmitLoadLocal(op); }
void Example::Emit(LoadLocal<u32>& op) { EmitLoadLocal(op); }
void Example::Emit(LoadLocal<i64>& op) { EmitLoadLocal(op); }
void Example::Emit(LoadLocal<u64>& op) { EmitLoadLocal(op); }
void Example::Emit(LoadLocal<float>& op) { EmitLoadLocal(op); }
void Example::Emit(LoadLocal<double>& op) { EmitLoadLocal(op); }
void Example::Emit(LoadLocal<Struct>& op) { EmitLoadLocal(op); }
void Example::Emit(LoadLocal<Pointer>& op) { EmitLoadLocal(op); }
void Example::Emit(StoreLocal<i8>& op) { EmitStoreLocal(op); }
void Example::Emit(StoreLocal<u8>& op) { EmitStoreLocal(op); }
void Example::Emit(StoreLocal<i16>& op) { EmitStoreLocal(op); }
void Example::Emit(StoreLocal<u16>& op) { EmitStoreLocal(op); }
void Example::Emit(StoreLocal<i32>& op) { EmitStoreLocal(op); }
void Example::Emit(StoreLocal<u32>& op) { EmitStoreLocal(op); }
void Example::Emit(StoreLocal<i64>& op) { EmitStoreLocal(op); }
void Example::Emit(StoreLocal<u64>& op) { EmitStoreLocal(op); }
void Example::Emit(StoreLocal<float>& op) { EmitStoreLocal(op); }
void Example::Emit(StoreLocal<double>& op) { EmitStoreLocal(op); }
void Example::Emit(StoreLocal<Struct>& op) { EmitStoreLocal(op); }
void Example::Emit(StoreLocal<Pointer>& op) { EmitStoreLocal(op); }
void Example::Emit(LoadFromPointer<i8>& op) { EmitLoadFromPointer(op); }
void Example::Emit(LoadFromPointer<u8>& op) { EmitLoadFromPointer(op); }
void Example::Emit(LoadFromPointer<i16>& op) { EmitLoadFromPointer(op); }
void Example::Emit(LoadFromPointer<u16>& op) { EmitLoadFromPointer(op); }
void Example::Emit(LoadFromPointer<i32>& op) { EmitLoadFromPointer(op); }
void Example::Emit(LoadFromPointer<u32>& op) { EmitLoadFromPointer(op); }
void Example::Emit(LoadFromPointer<i64>& op) { EmitLoadFromPointer(op); }
void Example::Emit(LoadFromPointer<u64>& op) { EmitLoadFromPointer(op); }
void Example::Emit(LoadFromPointer<float>& op) { EmitLoadFromPointer(op); }
void Example::Emit(LoadFromPointer<double>& op) { EmitLoadFromPointer(op); }
void Example::Emit(LoadFromPointer<Struct>& op) { EmitLoadFromPointer(op); }
void Example::Emit(LoadFromPointer<Pointer>& op) { EmitLoadFromPointer(op); }
void Example::Emit(StoreToPointer<i8>& op) { EmitStoreToPointer(op); }
void Example::Emit(StoreToPointer<u8>& op) { EmitStoreToPointer(op); }
void Example::Emit(StoreToPointer<i16>& op) { EmitStoreToPointer(op); }
void Example::Emit(StoreToPointer<u16>& op) { EmitStoreToPointer(op); }
void Example::Emit(StoreToPointer<i32>& op) { EmitStoreToPointer(op); }
void Example::Emit(StoreToPointer<u32>& op) { EmitStoreToPointer(op); }
void Example::Emit(StoreToPointer<i64>& op) { EmitStoreToPointer(op); }
void Example::Emit(StoreToPointer<u64>& op) { EmitStoreToPointer(op); }
void Example::Emit(StoreToPointer<float>& op) { EmitStoreToPointer(op); }
void Example::Emit(StoreToPointer<double>& op) { EmitStoreToPointer(op); }
void Example::Emit(StoreToPointer<Struct>& op) { EmitStoreToPointer(op); }
void Example::Emit(StoreToPointer<Pointer>& op) { EmitStoreToPointer(op); }
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