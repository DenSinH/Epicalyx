#pragma once

#include "calyx/backend/Backend.h"
#include "ProgramDependencies.h"
#include "Containers.h"
#include "CustomAssert.h"

#include <vector>


namespace epi {

using namespace calyx;

struct BasicOptimizer final : calyx::Backend {

  // todo: function level stuff
  BasicOptimizer(Function&& function) : 
      old_function{std::move(function)},
      old_deps{FunctionDependencies::GetDependencies(old_function)},
      new_function{old_function.symbol} {

  }

private:
  Function old_function;
  FunctionDependencies old_deps;

  calyx::Function new_function;
  cotyl::unordered_map<block_label_t, func_pos_t> block_links{};
  Graph<block_label_t, const block_t*, true> new_block_graph{};

  // direct variable replacements
  cotyl::unordered_map<var_index_t, var_index_t> var_replacement{};

  struct LocalData {
    var_index_t aliases = 0;                    // local might alias another local
    std::unique_ptr<calyx::Expr> replacement;   // replacement for LoadLocals
    pDirective store;                           // store to flush local with
  };

  // local replacements (loads/stores/alias loads/alias stores)
  cotyl::unordered_map<var_index_t, LocalData> locals{};

  void FlushLocal(var_index_t loc_idx, LocalData&& local);
  void FlushCurrentLocals();
  void FlushAliasedLocals();

  // variable found location in new program
  cotyl::unordered_map<calyx::var_index_t, std::pair<calyx::block_label_t, u64>> vars_found{};

  // current block that is being built
  calyx::block_t* current_block{};
  func_pos_t current_old_pos;            // position we are scanning in the old function
  block_label_t current_new_block_idx;      // block index we are building in the new program
  cotyl::unordered_set<block_label_t> todo{};
  bool reachable;

  template<typename T> 
  std::unique_ptr<T> CopyDirective(const T& directive) {
    return std::make_unique<T>(directive);
  }

  template<typename T>
  func_pos_t Output(std::unique_ptr<T>&& directive) {
    cotyl::Assert(reachable);
    const u64 in_block = current_block->size();
    current_block->push_back(std::move(directive));
    return std::make_pair(current_new_block_idx, in_block);
  }

  template<typename T, typename... Args>
  func_pos_t OutputNew(Args... args) {
    return Output(std::make_unique<T>(args...));
  }

  template<typename T>
  func_pos_t OutputCopy(const T& op) {
    return OutputNew<T>(op);
  }

  template<typename T, typename... Args>
  void OutputExprNew(calyx::var_index_t idx, const Args&... args) {
    vars_found.emplace(idx, OutputNew<T>(idx, args...));
  }

  template<typename T>
  void OutputExpr(std::unique_ptr<T>&& expr) {
    // expr will be moved before reading the idx on the return
    const auto idx = expr->idx;
    vars_found.emplace(idx, Output(std::move(expr)));
  }

  template<typename T>
  void OutputExprCopy(const T& expr) {
    vars_found.emplace(expr.idx, OutputCopy(expr));
  }

  template<typename T, typename... Args>
  void EmitRepl(Args... args) {
    auto repl = T(args...);
    Emit(repl);
  }

  // find a suitable replacement for some expression result
  // based on a predicate
  template<typename T, class F>
  bool FindExprResultReplacement(T& op, F predicate);

  // resolve branch indirections to single-branch blocks
  void ResolveBranchIndirection(block_label_t& dest);

  // link two blocks by "jumping" to that block and
  // emitting from there
  void LinkBlock(block_label_t next_block);

  // check whether predicate BadPred occurs before a GoodPred happens
  // on all forward paths (including loops) ...
  template<class BadPred, class GoodPred>
  bool NoBadBeforeGoodAllPaths(BadPred bad, GoodPred good, func_pos_t pos) const;

  // ... used for example to check whether we even need to flush a local,
  // or whether the value is overwritten anyway, before a write happens
  bool ShouldFlushLocal(var_index_t loc_idx, const LocalData& local);

  // try to get the expression directive a var was created with
  // return nullptr if it was not of this type
  template<typename T>
  const T* TryGetVarDirective(var_index_t idx) const;

  // try to replace a variable with a same-valued variable
  // tracked in the var_replacement map
  void TryReplaceVar(calyx::var_index_t& var_idx) const;

  // remove any unreachable block edges from the old_deps.block_graph
  // this is the ONLY function that affects the old dependencies.
  // Doing this improves block linking, speeding up optimization
  void RemoveUnreachableBlockEdges(block_label_t block);
  void RemoveUnreachableBlockEdgesRecurse(block_label_t block);

  // find common ancestor for 2 nodes such that all paths to these nodes go through that ancestor
  block_label_t CommonBlockAncestor(block_label_t first, block_label_t second) const;

public:
  Function&& Optimize();
  
  void Emit(const LoadLocalAddr& op) final;
  void Emit(const LoadGlobalAddr& op) final;

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