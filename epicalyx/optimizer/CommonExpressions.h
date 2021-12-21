#pragma once

#pragma once

#include "calyx/backend/Backend.h"
#include "ProgramDependencies.h"

#include <vector>
#include <unordered_map>
#include <set>

namespace epi {

using namespace calyx;

struct CommonExpressions : calyx::Backend {
  std::unordered_map<var_index_t, var_index_t> replacement{};

  ProgramDependencies dependencies{};
  calyx::Program new_program{};

  std::unordered_map<calyx::var_index_t, std::pair<calyx::block_label_t, u64>> vars_found{};
  calyx::Program::block_t* current_block{};
  block_label_t current_block_idx;

  template<typename T, class F>
  bool ReplaceIf(T& op, F predicate);

  std::pair<calyx::block_label_t, int> EmitNew(pDirective&& directive) {
    const u64 in_block = current_block->size();
    current_block->push_back(std::move(directive));
    return std::make_pair(current_block_idx, in_block);
  }

  void TryReplace(calyx::var_index_t& var_idx) const;

  // find common ancestor for 2 blocks such that all paths to these blocks go through that ancestor
  calyx::block_label_t CommonBlockAncestor(calyx::block_label_t first, calyx::block_label_t second) const;
  std::set<calyx::block_label_t> UpwardClosure(calyx::block_label_t base) const;

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