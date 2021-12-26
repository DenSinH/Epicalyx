#pragma once

#pragma once

#include "calyx/backend/Backend.h"
#include "ProgramDependencies.h"
#include "Containers.h"

#include <vector>


namespace epi {

using namespace calyx;

struct BasicOptimizer final : ProgramDependencies {
  using PD = ProgramDependencies;

  BasicOptimizer(const Program& program, ProgramDependencies&& deps) :
      program(program), ProgramDependencies(deps) {

  }

  const Program& program;

  calyx::Program new_program{};

  // direct variable replacements
  cotyl::unordered_map<var_index_t, var_index_t> var_replacement{};

  struct Local {
    var_index_t aliases = 0;                    // local might alias another local
    std::unique_ptr<calyx::Expr> replacement;   // replacement for LoadLocals
    pDirective store;                           // store to flush local with
  };

  // var local aliases
  cotyl::unordered_map<var_index_t, var_index_t> var_aliases{};
  // local replacements (loads/stores/alias loads/alias stores)
  cotyl::unordered_map<var_index_t, Local> locals{};

  void FlushLocal(var_index_t loc_idx, Local&& local);
  void FlushCurrentLocals();

  // variable found location in new program
  cotyl::unordered_map<calyx::var_index_t, std::pair<calyx::block_label_t, u64>> vars_found{};


  // current block that is being built
  calyx::Program::block_t* current_block{};
  block_label_t current_old_block_idx;      // block index we are scanning in the old program
  block_label_t current_new_block_idx;      // block index we are building in the new program
  bool conditional_branch_from;             // conditional branch has happened from current block
  cotyl::unordered_set<block_label_t> visited{};
  cotyl::unordered_set<block_label_t> todo{};
  bool reachable;

  template<typename T, class F>
  bool FindExprResultReplacement(T& op, F predicate);
  bool ResolveBranchIndirection(calyx::Branch& op);
  void LinkBlocks(block_label_t next_block);

  template<typename T>
  T* TryGetVarDirective(var_index_t idx);

  template<typename T, typename... Args>
  std::pair<calyx::block_label_t, int> EmitNew(Args... args) {
    cotyl::Assert(reachable);
    const u64 in_block = current_block->size();
    auto directive = std::make_unique<T>(args...);
    this->PD::Emit(*directive);
    current_block->push_back(std::move(directive));
    return std::make_pair(current_new_block_idx, in_block);
  }

  template<typename T>
  std::pair<calyx::block_label_t, int> EmitCopy(const T& op) {
    return EmitNew<T>(op);
  }

  template<typename T, typename... Args>
  void EmitExpr(calyx::var_index_t idx, const Args&... args) {
    var_graph[idx] = {};
    vars_found.emplace(idx, EmitNew<T>(idx, args...));
  }

  template<typename T>
  void EmitExprCopy(const T& expr) {
    var_graph[expr.idx] = {};
    vars_found.emplace(expr.idx, EmitNew<T>(expr));
  }

  void TryReplaceVar(calyx::var_index_t& var_idx) const;

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