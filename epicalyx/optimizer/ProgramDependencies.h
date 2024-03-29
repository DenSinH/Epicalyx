#pragma once

#include "calyx/backend/Backend.h"
#include "Containers.h"

#include <vector>

namespace epi {

using namespace calyx;

struct ProgramDependencies final : calyx::Backend {

  static ProgramDependencies GetDependencies(const Program& program) {
    auto deps = ProgramDependencies();
    deps.EmitProgram(program);
    return deps;
  }

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
    program_pos_t pos_made = {0, -1};
    cotyl::unordered_set<var_index_t> deps{};
    cotyl::unordered_set<var_index_t> read_for{};
    cotyl::unordered_set<program_pos_t> other_uses{};  // call arg/return val/store to pointer etc.
  };

  cotyl::unordered_map<var_index_t, Var> var_graph{};

  struct Local {
    using Write = std::pair<program_pos_t, var_index_t>;

    program_pos_t pos_made = {0, -1};
    cotyl::unordered_set<Write> writes{};
    cotyl::unordered_set<program_pos_t> reads{};
  };

  cotyl::unordered_map<var_index_t, Local> local_graph{};

  program_pos_t pos;

  void VisualizeVars();

  void EmitProgram(const Program& program) final;

  void Emit(const AllocateLocal& op) final;
  void Emit(const DeallocateLocal& op) final;
  void Emit(const LoadLocalAddr& op) final;
  void Emit(const LoadGlobalAddr& op) final;
  void Emit(const ArgMakeLocal& op) final;

  template<typename To, typename From>
  void EmitCast(const Cast<To, From>& op);
  template<typename T>
  void EmitLoadLocal(const LoadLocal<T>& op);
  template<typename T>
  void EmitStoreLocal(const StoreLocal<T>& op);
  template<typename T>
  void EmitLoadGlobal(const LoadGlobal<T>& op);
  template<typename T>
  void EmitStoreGlobal(const StoreGlobal<T>& op);
  template<typename T>
  void EmitLoadFromPointer(const LoadFromPointer<T>& op);
  template<typename T>
  void EmitStoreToPointer(const StoreToPointer<T>& op);
  template<typename T>
  void EmitCall(const Call<T>& op);
  template<typename T>
  void EmitCallLabel(const CallLabel<T>& op);
  template<typename T>
  void EmitReturn(const Return<T>& op);
  template<typename T>
  void EmitImm(const Imm<T>& op);
  template<typename T>
  void EmitUnop(const Unop<T>& op);
  template<typename T>
  void EmitBinop(const Binop<T>& op);
  template<typename T>
  void EmitBinopImm(const BinopImm<T>& op);
  template<typename T>
  void EmitShift(const Shift<T>& op);
  template<typename T>
  void EmitShiftImm(const ShiftImm<T>& op);
  template<typename T>
  void EmitCompare(const Compare<T>& op);
  template<typename T>
  void EmitCompareImm(const CompareImm<T>& op);
  template<typename T>
  void EmitBranchCompare(const BranchCompare<T>& op);
  template<typename T>
  void EmitBranchCompareImm(const BranchCompareImm<T>& op);
  template<typename T>
  void EmitAddToPointer(const AddToPointer<T>& op);

#include "calyx/backend/Methods.inl"

};

}