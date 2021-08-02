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

void Example::Emit(Return& op) {

}

template<typename To, typename From>
void Example::EmitCast(Cast<To, From>& op) {

}

template<typename T>
void Example::EmitStoreCVar(StoreCVar<T>& op) {

}

template<typename T>
void Example::EmitLoadCVar(LoadCVar<T>& op) {

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


}