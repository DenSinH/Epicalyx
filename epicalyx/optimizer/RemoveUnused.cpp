#include "RemoveUnused.h"
#include "calyx/Calyx.h"
#include "ProgramDependencies.h"

#include <algorithm>


namespace epi {

static void NullifyUnusedLocals(calyx::Function& func, FunctionDependencies& deps) {
  // remove unused locals
  for (const auto& [loc_idx, local] : deps.local_graph) {
    if (local.reads.empty()) {
      // local is never read/aliased
      // remove all local writes
      for (const auto& pos : local.writes) {
        func.blocks.at(pos.first)[pos.second] = nullptr;
      }
      func.locals.erase(loc_idx);
    }
  }
}

static void NullifyUnusedVars(calyx::Function& function, FunctionDependencies& dependencies) {
  cotyl::unordered_set<var_index_t> todo{};
  // copy map keys
  std::transform(dependencies.var_graph.begin(), dependencies.var_graph.end(), std::inserter(todo, todo.begin()),
                 [](auto& kv) { return kv.first; });

  while (!todo.empty()) {
    const auto var_idx = *todo.begin();
    todo.erase(todo.begin());

    // remove nullified reads
    auto& var = dependencies.var_graph.at(var_idx);
    std::erase_if(var.reads, [&](const auto& pos) { return function.blocks.at(pos.first).at(pos.second) == nullptr; });

    // NEVER erase call results
    if (var.reads.empty() && !var.is_call_result) {
      // remove from all dependencies
      for (auto other : var.deps) {
        // other var is now also possibly unused
        todo.insert(other);
      }

      // nullify write
      function.blocks.at(var.created.first)[var.created.second] = nullptr;
    }
  }
}

size_t RemoveUnused(calyx::Function& function) {
  auto deps = FunctionDependencies::GetDependencies(function);

  // this also removed any uses from the stored vars
  NullifyUnusedLocals(function, deps);
  NullifyUnusedVars(function, deps);

  // remove nullified directives
  size_t removed_directives = 0;
  for (auto& [block_idx, block] : function.blocks) {
    removed_directives += std::erase(block, nullptr);
  }
  return removed_directives;
}

}