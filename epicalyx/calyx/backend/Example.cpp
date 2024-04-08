#include "Example.h"


namespace epi::calyx {

void Example::EmitProgram(const Program& program) {

}

template<typename To, typename From>
void Example::EmitCast(const Cast<To, From>& op) {

}

template<typename T>
void Example::EmitLoadLocal(const LoadLocal<T>& op) {

}

void Example::Emit(const LoadLocalAddr& op) {

}

template<typename T>
void Example::EmitStoreLocal(const StoreLocal<T>& op) {

}

template<typename T>
void Example::EmitLoadGlobal(const LoadGlobal<T>& op) {

}

void Example::Emit(const LoadGlobalAddr& op) {

}

template<typename T>
void Example::EmitStoreGlobal(const StoreGlobal<T>& op) {

}

template<typename T>
void Example::EmitLoadFromPointer(const LoadFromPointer<T>& op) {
  
}

template<typename T>
void Example::EmitStoreToPointer(const StoreToPointer<T>& op) {
  
}

template<typename T>
void Example::EmitCall(const Call<T>& op) {

}

template<typename T>
void Example::EmitCallLabel(const CallLabel<T>& op) {

}

template<typename T>
void Example::EmitReturn(const Return<T>& op) {

}

template<typename T>
void Example::EmitImm(const Imm<T>& op) {

}

template<typename T>
void Example::EmitUnop(const Unop<T>& op) {

}

template<typename T>
void Example::EmitBinop(const Binop<T>& op) {

}

template<typename T>
void Example::EmitBinopImm(const BinopImm<T>& op) {

}

template<typename T>
void Example::EmitShift(const Shift<T>& op) {

}

template<typename T>
void Example::EmitShiftImm(const ShiftImm<T>& op) {

}

template<typename T>
void Example::EmitCompare(const Compare<T>& op) {

}

template<typename T>
void Example::EmitCompareImm(const CompareImm<T>& op) {

}

void Example::Emit(const UnconditionalBranch& op) {

}

template<typename T>
void Example::EmitBranchCompare(const BranchCompare<T>& op) {

}

template<typename T>
void Example::EmitBranchCompareImm(const BranchCompareImm<T>& op) {

}

void Example::Emit(const Select& op) {

}

template<typename T>
void Example::EmitAddToPointer(const AddToPointer<T>& op) {

}

void Example::Emit(const AddToPointerImm& op) {

}

#define BACKEND_NAME Example
#include "Templates.inl"

}