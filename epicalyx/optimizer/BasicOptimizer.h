#pragma once

#include "calyx/backend/Backend.h"
#include "ProgramDependencies.h"
#include "Containers.h"
#include "CustomAssert.h"

#include <vector>


namespace epi {

using namespace calyx;

struct BasicOptimizer final : calyx::Backend {

  BasicOptimizer(const Program& program) :
      program(program), deps{ProgramDependencies::GetDependencies(program)} {

  }

  const Program& program;
  ProgramDependencies deps;

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
  cotyl::unordered_set<block_label_t> visited{};
  cotyl::unordered_set<block_label_t> todo{};
  bool reachable;

  template<typename T, class F>
  bool FindExprResultReplacement(T& op, F predicate);
  bool ResolveBranchIndirection(calyx::Branch& op);
  void LinkBlock(block_label_t next_block);

  template<typename T>
  const T* TryGetVarDirective(var_index_t idx) const;

  template<typename T> 
  std::unique_ptr<T> CopyDirective(const T& directive) {
    return std::make_unique<T>(directive);
  }

  template<typename T>
  program_pos_t Output(std::unique_ptr<T>&& directive) {
    cotyl::Assert(reachable);
    const u64 in_block = current_block->size();
    deps.Emit(*directive);
    current_block->push_back(std::move(directive));
    return std::make_pair(current_new_block_idx, in_block);
  }

  template<typename T, typename... Args>
  program_pos_t OutputNew(Args... args) {
    return Output(std::make_unique<T>(args...));
  }

  template<typename T>
  program_pos_t OutputCopy(const T& op) {
    return OutputNew<T>(op);
  }

  template<typename T, typename... Args>
  void OutputExprNew(calyx::var_index_t idx, const Args&... args) {
    deps.var_graph[idx] = {};
    vars_found.emplace(idx, OutputNew<T>(idx, args...));
  }

  template<typename T>
  void OutputExpr(std::unique_ptr<T>&& expr) {
    // expr will be moved before reading the idx on the return
    const auto idx = expr->idx;
    deps.var_graph[idx] = {};
    vars_found.emplace(idx, Output(std::move(expr)));
  }

  template<typename T>
  void OutputExprCopy(const T& expr) {
    deps.var_graph[expr.idx] = {};
    vars_found.emplace(expr.idx, OutputCopy(expr));
  }

  template<typename T, typename... Args>
  void EmitRepl(Args... args) {
    auto repl = T(args...);
    Emit(repl);
  }

  void TryReplaceVar(calyx::var_index_t& var_idx) const;

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