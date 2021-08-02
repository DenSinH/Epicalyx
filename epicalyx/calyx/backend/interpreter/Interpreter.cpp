#include "Interpreter.h"


namespace epi::calyx {


void Interpreter::EmitProgram(std::vector<calyx::pDirective>& program) {
  for (auto& directive : program) {
    directive->Emit(*this);
  }
}

void Interpreter::Emit(AllocateCVar& op) {

}

void Interpreter::Emit(DeallocateCVar& op) {

}

void Interpreter::Emit(LoadCVarAddr& op) {

}

void Interpreter::Emit(Return& op) {

}

template<typename To, typename From>
void Interpreter::EmitCast(Cast<To, From>& op) {

}

template<typename T>
void Interpreter::EmitStoreCVar(StoreCVar<T>& op) {

}

template<typename T>
void Interpreter::EmitLoadCVar(LoadCVar<T>& op) {

}

template<typename T>
void Interpreter::EmitImm(Imm<T>& op) {

}

template<typename T>
void Interpreter::EmitUnop(Unop<T>& op) {

}

template<typename T>
void Interpreter::EmitBinop(Binop<T>& op) {

}

template<typename T>
void Interpreter::EmitAddToPointer(AddToPointer<T>& op) {

}


}