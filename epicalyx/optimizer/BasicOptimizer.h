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
      new_function{old_function.symbol} {

  }

  calyx::Function&& Optimize();

private:
  calyx::Function old_function;
  FunctionDependencies old_deps;

  calyx::Function new_function;
  
  // current block that is being built
  calyx::block_t* current_block{};
  func_pos_t current_old_pos;            // position we are scanning in the old function
  block_label_t current_new_block_idx;      // block index we are building in the new program
  cotyl::unordered_set<block_label_t> todo{};
  bool reachable;

  template<typename T> 
  T CopyDirective(const T& directive) {
    return directive;
  }

  template<typename T>
  func_pos_t Output(T&& directive) {
    cotyl::Assert(reachable);
    const u64 in_block = current_block->size();
    current_block->push_back(T{std::move(directive)});
    return std::make_pair(current_new_block_idx, in_block);
  }

  template<typename T, typename... Args>
  func_pos_t OutputNew(Args... args) {
    return Output(T{args...});
  }

  template<typename T>
  func_pos_t OutputCopy(const T& op) {
    return OutputNew<T>(op);
  }

  template<typename T, typename... Args>
  void OutputExprNew(var_index_t idx, const Args&... args) {
    vars_found.emplace(idx, OutputNew<T>(idx, args...));
  }

  template<typename T>
  requires (std::is_base_of_v<calyx::Expr, T>)
  void OutputExpr(T&& expr) {
    // expr will be moved before reading the idx on the return
    const auto idx = expr.idx;
    vars_found.emplace(idx, Output(std::move(expr)));
  }

  template<typename T>
  requires (std::is_base_of_v<calyx::Expr, T>)
  void OutputExprCopy(const T& expr) {
    vars_found.emplace(expr.idx, OutputCopy(expr));
  }

  template<typename T, typename... Args>
  void EmitRepl(Args... args) {
    auto repl = T(args...);
    Emit(repl);
  }

  cotyl::unordered_map<block_label_t, func_pos_t> block_links{};
  Graph<block_label_t, const calyx::block_t*, true> new_block_graph{};

  // direct variable replacements
  cotyl::unordered_map<var_index_t, var_index_t> var_replacement{};

  // variable found location in new program
  cotyl::unordered_map<var_index_t, std::pair<block_label_t, u64>> vars_found{};

  // find a suitable replacement for some expression result
  // based on a predicate
  template<typename T, class F>
  bool FindExprResultReplacement(T& op, F predicate);

  // resolve branch indirections to single-branch blocks
  void ResolveBranchIndirection(block_label_t& dest) const;

  // resolve block links from old block labels to new block labels
  void ResolveBlockLinks(block_label_t& dest) const;

  // link two blocks by "jumping" to that block and
  // emitting from there
  void LinkBlock(block_label_t next_block);

  using local_replacement_t = std::shared_ptr<calyx::AnyExpr>;
  struct LocalData {
    var_index_t aliases = 0;                     // local might alias another local
    local_replacement_t replacement;             // replacement for LoadLocals
    std::optional<calyx::AnyDirective> store;    // store to flush local with
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
  
  void Emit(const calyx::AnyDirective& dir) {
    dir.template visit<void>([&](const auto& d) { Emit(d); });
  }

  void Emit(const calyx::AnyExpr& expr) {
    expr.template visit<void>([&](const auto& e) { Emit(e); });
  }

  template<typename To, typename From>
  void Emit(const calyx::Cast<To, From>& op);
  template<typename T>
  void Emit(const calyx::LoadLocal<T>& op);
  void Emit(const calyx::LoadLocalAddr& op);
  template<typename T>
  void Emit(const calyx::StoreLocal<T>& op);
  template<typename T>
  void Emit(const calyx::LoadGlobal<T>& op);
  void Emit(const calyx::LoadGlobalAddr& op);
  template<typename T>
  void Emit(const calyx::StoreGlobal<T>& op);
  template<typename T>
  void Emit(const calyx::LoadFromPointer<T>& op);
  template<typename T>
  void Emit(const calyx::StoreToPointer<T>& op);
  template<typename T>
  void Emit(const calyx::AddToPointer<T>& op);
  template<typename T>
  void Emit(const calyx::Call<T>& op);
  template<typename T>
  void Emit(const calyx::CallLabel<T>& op);
  template<typename T>
  void Emit(const calyx::Return<T>& op);
  template<typename T>
  void Emit(const calyx::Imm<T>& op);
  template<typename T>
  void Emit(const calyx::Unop<T>& op);
  template<typename T>
  void Emit(const calyx::Binop<T>& op);
  template<typename T>
  void Emit(const calyx::Shift<T>& op);
  template<typename T>
  void Emit(const calyx::Compare<T>& op);
  template<typename T>
  void Emit(const calyx::BranchCompare<T>& op);
  void Emit(const calyx::UnconditionalBranch& op);
  void Emit(const calyx::Select& op);
};

}