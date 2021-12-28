#pragma once

#include "calyx/backend/Backend.h"
#include "Containers.h"

#include <vector>

namespace epi {

using namespace calyx;

struct ProgramDependencies : calyx::Backend {

  // find common ancestor for 2 blocks such that all paths to these blocks go through that ancestor
  calyx::block_label_t CommonBlockAncestor(calyx::block_label_t first, calyx::block_label_t second) const;
  std::vector<calyx::block_label_t> OrderedUpwardClosure(calyx::block_label_t base) const;
  cotyl::unordered_set<calyx::block_label_t> UpwardClosure(cotyl::unordered_set<calyx::block_label_t>&& base) const;
  bool IsAncestorOf(calyx::block_label_t base, calyx::block_label_t other) const;

  struct Block {
    cotyl::unordered_set<block_label_t> to{};
    cotyl::unordered_set<block_label_t> from{};
  };

  cotyl::unordered_map<block_label_t, Block> block_graph{};

  struct Var {
    std::pair<block_label_t, int> pos_made = {0, -1};
    cotyl::unordered_set<var_index_t> deps{};
    cotyl::unordered_set<var_index_t> read_for{};
    u64 other_uses = 0;  // call arg/return val/store to pointer etc.
  };

  cotyl::unordered_map<var_index_t, Var> var_graph{};

  struct Local {
    using Write = std::pair<std::pair<block_label_t, int>, var_index_t>;

    std::pair<block_label_t, int> pos_made = {0, -1};
    cotyl::unordered_set<Write> writes{};
    u64 reads = 0;
  };

  cotyl::unordered_map<var_index_t, Local> local_graph{};

  std::pair<block_label_t, int> pos;

  void VisualizeVars();

  void EmitProgram(Program& program) override;

  void Emit(AllocateLocal& op) override;
  void Emit(DeallocateLocal& op) override;
  void Emit(LoadLocalAddr& op) override;
  void Emit(LoadGlobalAddr& op) override;
  void Emit(ArgMakeLocal& op) override;

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

#define final override
#include "calyx/backend/Methods.inl"
#undef final

};

}