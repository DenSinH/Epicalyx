#pragma once

#include "calyx/backend/Backend.h"
#include "Containers.h"

#include <vector>

namespace epi {

using namespace calyx;

struct ProgramDependencies : calyx::Backend {

  // find common ancestor for 2 blocks such that all paths to these blocks go through that ancestor
  calyx::block_label_t CommonBlockAncestor(calyx::block_label_t first, calyx::block_label_t second) const;
  std::vector<calyx::block_label_t> UpwardClosure(calyx::block_label_t base) const;
  bool IsAncestorOf(calyx::block_label_t base, calyx::block_label_t other) const;

  struct Edge {
    cotyl::unordered_set<block_label_t> to{};
    cotyl::unordered_set<block_label_t> from{};
  };

  cotyl::unordered_map<block_label_t, Edge> block_graph{};

  struct Var {
    block_label_t block_made = -1;
    cotyl::unordered_set<var_index_t> deps{};
    u64 read_count = 0;
  };

  cotyl::unordered_map<var_index_t, Var> var_graph{};

  std::pair<block_label_t, size_t> pos{};

  void VisualizeVars();

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