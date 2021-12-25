#include "RemoveUnused.h"
#include "IRCompare.h"
#include "Cast.h"


namespace epi {

void RemoveUnused::EmitProgram(Program& program) {
  dependencies.EmitProgram(program);
  new_program.functions    = std::move(program.functions);
  new_program.globals      = std::move(program.globals);
  new_program.local_labels = std::move(program.local_labels);
  new_program.strings      = std::move(program.strings);

  for (const auto& [symbol, entry] : new_program.functions) {
    auto closure = dependencies.UpwardClosure(entry);

    for (const auto& block : closure) {
      auto inserted = new_program.blocks.emplace(block, calyx::Program::block_t{}).first;
      current_block = &inserted->second;
      for (const auto& directive : program.blocks.at(block)) {
        directive->Emit(*this);
      }
    }
  }
}

void RemoveUnused::Emit(AllocateLocal& op) {
  EmitCopy(op);
}

void RemoveUnused::Emit(DeallocateLocal& op) {
  EmitCopy(op);
}

template<typename To, typename From>
void RemoveUnused::EmitCast(Cast<To, From>& op) {
  EmitExpr(op);
}

template<typename T>
void RemoveUnused::EmitLoadLocal(LoadLocal<T>& op) {
  EmitExpr(op);
}

void RemoveUnused::Emit(LoadLocalAddr& op) {
  EmitExpr(op);
}

template<typename T>
void RemoveUnused::EmitStoreLocal(StoreLocal<T>& op) {
  EmitCopy(op);
}

template<typename T>
void RemoveUnused::EmitLoadGlobal(LoadGlobal<T>& op) {
  EmitExpr(op);
}

void RemoveUnused::Emit(LoadGlobalAddr& op) {
  EmitExpr(op);
}

template<typename T>
void RemoveUnused::EmitStoreGlobal(StoreGlobal<T>& op) {
  EmitCopy(op);
}

template<typename T>
void RemoveUnused::EmitLoadFromPointer(LoadFromPointer<T>& op) {
  EmitExpr(op);
}

template<typename T>
void RemoveUnused::EmitStoreToPointer(StoreToPointer<T>& op) {
  EmitCopy(op);
}

template<typename T>
void RemoveUnused::EmitCall(Call<T>& op) {
  EmitCopy(op);
}

template<typename T>
void RemoveUnused::EmitCallLabel(CallLabel<T>& op) {
  EmitCopy(op);
}

void RemoveUnused::Emit(ArgMakeLocal& op) {
  EmitCopy(op);
}

template<typename T>
void RemoveUnused::EmitReturn(Return<T>& op) {
  EmitCopy(op);
}

template<typename T>
void RemoveUnused::EmitImm(Imm<T>& op) {
  EmitExpr(op);
}

template<typename T>
void RemoveUnused::EmitUnop(Unop<T>& op) {
  EmitExpr(op);
}

template<typename T>
void RemoveUnused::EmitBinop(Binop<T>& op) {
  EmitExpr(op);
}

template<typename T>
void RemoveUnused::EmitBinopImm(BinopImm<T>& op) {
  EmitExpr(op);
}

template<typename T>
void RemoveUnused::EmitShift(Shift<T>& op) {
  EmitExpr(op);
}

template<typename T>
void RemoveUnused::EmitShiftImm(ShiftImm<T>& op) {
  EmitExpr(op);
}

template<typename T>
void RemoveUnused::EmitCompare(Compare<T>& op) {
  EmitExpr(op);
}

template<typename T>
void RemoveUnused::EmitCompareImm(CompareImm<T>& op) {
  EmitExpr(op);
}

void RemoveUnused::Emit(UnconditionalBranch& op) {
  EmitCopy(op);
}

template<typename T>
void RemoveUnused::EmitBranchCompare(BranchCompare<T>& op) {
  EmitCopy(op);
}

template<typename T>
void RemoveUnused::EmitBranchCompareImm(BranchCompareImm<T>& op) {
  EmitCopy(op);
}

void RemoveUnused::Emit(Select& op) {
  EmitCopy(op);
}

template<typename T>
void RemoveUnused::EmitAddToPointer(AddToPointer<T>& op) {
  EmitExpr(op);
}

void RemoveUnused::Emit(AddToPointerImm& op) {
  EmitExpr(op);
}

#define BACKEND_NAME RemoveUnused
#include "calyx/backend/Templates.inl"

}