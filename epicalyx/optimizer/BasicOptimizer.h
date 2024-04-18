#pragma once

#include "calyx/Calyx.h"
#include "ProgramDependencies.h"
#include "Containers.h"
#include "CustomAssert.h"

#include <vector>


namespace epi {

struct BasicOptimizer {

  BasicOptimizer(calyx::Function&& function) : 
      old_function{std::move(function)},
      old_deps{FunctionDependencies::GetDependencies(old_function)},
      new_function{std::move(old_function.symbol)} {

  }

  calyx::Function&& Optimize();

private:
  calyx::Function old_function;
  FunctionDependencies old_deps;

  calyx::Function new_function;
  
  // current block that is being built
  calyx::BasicBlock* current_block{};
  func_pos_t current_old_pos;            // position we are scanning in the old function
  block_label_t current_new_block_idx;      // block index we are building in the new program
  cotyl::unordered_set<block_label_t> todo{};
  bool reachable;

  template<typename T> 
  requires (calyx::is_directive_v<T>)
  T CopyDirective(const T& directive);

  func_pos_t OutputAnyUnsafe(calyx::AnyDirective&& dir);

  template<typename T>
  requires (calyx::is_directive_v<T>)
  func_pos_t Output(T&& directive);

  template<typename T, typename... Args>
  requires (calyx::is_directive_v<T>)
  func_pos_t OutputNew(Args&&... args);

  template<typename T>
  requires (calyx::is_directive_v<T>)
  func_pos_t OutputCopy(const T& op);

  template<typename T, typename... Args>
  requires (calyx::is_expr_v<T>)
  void OutputExprNew(var_index_t idx, const Args&... args);

  template<typename T>
  requires (calyx::is_expr_v<T>)
  void OutputExpr(T&& expr);

  template<typename T, typename... Args>
  requires (calyx::is_directive_v<T>)
  void EmitRepl(Args&&... args);

  cotyl::unordered_map<block_label_t, func_pos_t> block_links{};
  Graph<block_label_t, const calyx::BasicBlock*, true> new_block_graph{};

  // direct variable replacements
  cotyl::unordered_map<var_index_t, var_index_t> var_replacement{};

  // variable found location in new program
  cotyl::unordered_map<var_index_t, std::pair<block_label_t, u64>> vars_found{};

  // find a suitable replacement for some expression result
  // based on a predicate
  template<typename T, class F>
  bool FindExprResultReplacement(T& op, F predicate);

  // resolve block links from old block labels to new block labels
  void ResolveBlockLinks(block_label_t& dest) const;

  // link two blocks by "jumping" to that block and
  // emitting from there
  void LinkBlock(block_label_t next_block);

  using local_replacement_t = std::shared_ptr<calyx::AnyExpr>;
  struct LocalData {
    var_index_t aliases = 0;                     // local might alias another local
    local_replacement_t replacement;             // replacement for LoadLocals
    std::unique_ptr<calyx::AnyDirective> store;  // store to flush local with
  };

  // local replacements (loads/stores/alias loads/alias stores)
  cotyl::unordered_map<var_index_t, LocalData> locals{};

  // local replacements in next block
  using local_values_t = cotyl::unordered_map<var_index_t, local_replacement_t>;
  cotyl::unordered_map<block_label_t, local_values_t> local_initial_values{};
  cotyl::unordered_map<block_label_t, local_values_t> local_final_values{};

  // flush all current locals
  // i.e., emit their writes and store their substitutions
  // for local propagation
  void FlushOnBranch();

  // emit a branch type instruction, flush locals and set state
  // also constructs the new_block_graph
  template<typename T>
  requires (std::is_base_of_v<calyx::Branch, T>)
  void DoBranch(T&& branch);

  // resolve branch indirections to single-branch blocks
  void ResolveBranchIndirection(block_label_t& dest) const;
  void RegisterBranchDestination(block_label_t& dest);

  // Flush current aliased locals
  // to be executed on instructions that may invalidate
  // the local state (i.e. a pointer write/read or a call)
  // as these may update the local's value and a
  // later flush of the write operation (or any reads
  // in the function call) may retrieve the wrong value
  void FlushAliasedLocals();

  // propagate local values when starting a new block
  void PropagateLocalValues();

  // store local data on write
  // this delays the write and instead saves it to the local
  // replacements, to be flushed / stored for propagation later
  void StoreLocalData(var_index_t loc_idx, LocalData&& local);

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
  void TryReplaceVar(var_index_t& var_idx) const;

  template<typename T>
  void TryReplaceOperand(calyx::Operand<T>& var) const;

  // remove any unreachable block edges from the old_deps.block_graph
  // this is the ONLY function that affects the old dependencies.
  // Doing this improves block linking, speeding up optimization
  void RemoveUnreachableBlockEdges(block_label_t block);
  void RemoveUnreachableBlockEdgesRecurse(block_label_t block);

  // find common ancestor for 2 nodes such that all paths to these nodes go through that ancestor
  block_label_t CommonBlockAncestor(block_label_t first, block_label_t second) const;
  
  void EmitDirective(const calyx::AnyDirective& dir);
  void EmitExpr(const calyx::AnyExpr& expr);

private:
  template<typename T>
  requires (calyx::is_directive_v<T>)
  void EmitGeneric(T&& op);
  void Emit(calyx::NoOp&& op) { }
  template<typename To, typename From>
  void Emit(calyx::Cast<To, From>&& op);
  template<typename T>
  void Emit(calyx::LoadLocal<T>&& op);
  void Emit(calyx::LoadLocalAddr&& op);
  template<typename T>
  void Emit(calyx::StoreLocal<T>&& op);
  template<typename T>
  void Emit(calyx::LoadGlobal<T>&& op);
  void Emit(calyx::LoadGlobalAddr&& op);
  template<typename T>
  void Emit(calyx::StoreGlobal<T>&& op);
  template<typename T>
  void Emit(calyx::LoadFromPointer<T>&& op);
  template<typename T>
  void Emit(calyx::StoreToPointer<T>&& op);
  template<typename T>
  void Emit(calyx::AddToPointer<T>&& op);
  template<typename T>
  void Emit(calyx::Call<T>&& op);
  template<typename T>
  void Emit(calyx::CallLabel<T>&& op);
  template<typename T>
  void Emit(calyx::Return<T>&& op);
  template<typename T>
  void Emit(calyx::Imm<T>&& op);
  template<typename T>
  void Emit(calyx::Unop<T>&& op);
  template<typename T>
  void Emit(calyx::Binop<T>&& op);
  template<typename T>
  void Emit(calyx::Shift<T>&& op);
  template<typename T>
  void Emit(calyx::Compare<T>&& op);
  template<typename T>
  void Emit(calyx::BranchCompare<T>&& op);
  void Emit(calyx::UnconditionalBranch&& op);
  void Emit(calyx::Select&& op);
};

}