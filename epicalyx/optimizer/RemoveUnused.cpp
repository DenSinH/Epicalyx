#include "RemoveUnused.h"
#include "calyx/Calyx.h"
#include "ProgramDependencies.h"

#include <algorithm>


namespace epi {

static void NullifyUnusedLocals(calyx::Function& func, FunctionDependencies& deps) {
  std::vector<var_index_t> to_remove{};

  // remove unused locals
  for (const auto& [loc_idx, local] : func.locals) {
    if (deps.local_graph.contains(loc_idx)) {
      const auto& loc_data = deps.local_graph.at(loc_idx);
      bool local_unread = std::all_of(loc_data.reads.begin(), loc_data.reads.end(), [&](const auto& pos) {
        return func.blocks.at(pos.first)[pos.second] == nullptr;
      });
      if (local_unread) {
        // local is never read/aliased
        // remove all local writes
        for (const auto& pos : loc_data.writes) {
          func.blocks.at(pos.first)[pos.second] = nullptr;
        }
        to_remove.emplace_back(loc_idx);
      }
    }
    else if (!local.arg_idx.has_value()) {
      to_remove.emplace_back(loc_idx);
    }
  }

  for (const auto& loc_idx : to_remove) {
    func.locals.erase(loc_idx);
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