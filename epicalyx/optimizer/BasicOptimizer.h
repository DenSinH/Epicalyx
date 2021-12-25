#pragma once

#pragma once

#include "calyx/backend/Backend.h"
#include "ProgramDependencies.h"

#include <vector>
#include <unordered_map>
#include <set>

namespace epi {

using namespace calyx;

struct BasicOptimizer : calyx::Backend {

  BasicOptimizer(const Program& program) : program(program) {

  }

  const Program& program;

  std::unordered_map<var_index_t, var_index_t> replacement{};

  ProgramDependencies dependencies{};
  calyx::Program new_program{};

  std::unordered_map<calyx::var_index_t, std::pair<calyx::block_label_t, u64>> vars_found{};
  calyx::Program::block_t* current_block{};
  block_label_t current_block_idx;

  template<typename T, class F>
  bool FindExprResultReplacement(T& op, F predicate);
  bool ResolveBranchIndirection(calyx::Branch& op);

  template<typename T, typename... Args>
  std::pair<calyx::block_label_t, int> EmitNew(Args... args) {
    const u64 in_block = current_block->size();
    current_block->push_back(std::make_unique<T>(args...));
    return std::make_pair(current_block_idx, in_block);
  }

  template<typename T, typename... Args>
  void EmitExpr(calyx::var_index_t idx, const Args&... args) {
    vars_found.emplace(idx, EmitNew<T>(idx, args...));
  }

  template<typename T>
  void EmitExprCopy(const T& expr) {
    vars_found.emplace(expr.idx, EmitNew<T>(expr));
  }

  void TryReplace(calyx::var_index_t& var_idx) const;

  void EmitProgram(Program& program) final;

  void Emit(AllocateLocal& op) final;
  void Emit(DeallocateLocal& op) final;
  void Emit(LoadLocalAddr& op) final;
  void Emit(LoadGlobalAddr& op) final;
  void Emit(ArgMakeLocal& op) final;

  template<typename To, typename From>
  void EmitCast(Cast<To, From>& op);
  template<typename T>
  void EmitLoadLocal(LoadLocal<T>& op);
  template<typename T>
  void EmitStoreLocal(StoreLocal<T>& op);
  template<typename T>
  void EmitLoadGlobal(LoadGlobal<T>& op);
  template<typename T>
  void EmitStoreGlobal(StoreGlobal<T>& op);
  template<typename T>
  void EmitLoadFromPointer(LoadFromPointer<T>& op);
  template<typename T>
  void EmitStoreToPointer(StoreToPointer<T>& op);
  template<typename T>
  void EmitCall(Call<T>& op);
  template<typename T>
  void EmitCallLabel(CallLabel<T>& op);
  template<typename T>
  void EmitReturn(Return<T>& op);
  template<typename T>
  void EmitImm(Imm<T>& op);
  template<typename T>
  void EmitUnop(Unop<T>& op);
  template<typename T>
  void EmitBinop(Binop<T>& op);
  template<typename T>
  void EmitBinopImm(BinopImm<T>& op);
  template<typename T>
  void EmitShift(Shift<T>& op);
  template<typename T>
  void EmitShiftImm(ShiftImm<T>& op);
  template<typename T>
  void EmitCompare(Compare<T>& op);
  template<typename T>
  void EmitCompareImm(CompareImm<T>& op);
  template<typename T>
  void EmitBranchCompare(BranchCompare<T>& op);
  template<typename T>
  void EmitBranchCompareImm(BranchCompareImm<T>& op);
  template<typename T>
  void EmitAddToPointer(AddToPointer<T>& op);

#include "calyx/backend/Methods.inl"

};

}