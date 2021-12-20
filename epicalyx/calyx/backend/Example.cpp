#include "Example.h"


namespace epi::calyx {

void Example::EmitProgram(Program& program) {

}

void Example::Emit(AllocateLocal& op) {

}

void Example::Emit(DeallocateLocal& op) {

}

template<typename To, typename From>
void Example::EmitCast(Cast<To, From>& op) {

}

template<typename T>
void Example::EmitLoadLocal(LoadLocal<T>& op) {

}

void Example::Emit(LoadLocalAddr& op) {

}

template<typename T>
void Example::EmitStoreLocal(StoreLocal<T>& op) {

}

template<typename T>
void Example::EmitLoadGlobal(LoadGlobal<T>& op) {

}

void Example::Emit(LoadGlobalAddr& op) {

}

template<typename T>
void Example::EmitStoreGlobal(StoreGlobal<T>& op) {

}

template<typename T>
void Example::EmitLoadFromPointer(LoadFromPointer<T>& op) {
  
}

template<typename T>
void Example::EmitStoreToPointer(StoreToPointer<T>& op) {
  
}

template<typename T>
void Example::EmitCall(Call<T>& op) {

}

template<typename T>
void Example::EmitCallLabel(CallLabel<T>& op) {

}

void Example::Emit(ArgMakeLocal& op) {

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

#define BACKEND_NAME Example
#include "Templates.inl"

}