#include "RemoveUnused.h"
#include "calyx/Calyx.h"
#include "ProgramDependencies.h"

#include <algorithm>


namespace epi {

static std::size_t NullifyUnusedLocals(calyx::Function& func, FunctionDependencies& deps) {
  // remove unused locals
  std::size_t removed = 0;
  for (const auto& [loc_idx, local] : deps.local_graph) {
    bool local_unread = std::all_of(local.reads.begin(), local.reads.end(), [&](const auto& pos) {
      return calyx::IsType<calyx::NoOp>(func.blocks.at(pos.first).at(pos.second));
    });
    if (local_unread) {
      // local is never read/aliased
      // remove all local writes
      for (const auto& pos : local.writes) {
        func.blocks.at(pos.first).at(pos.second).template emplace<calyx::NoOp>();
      }
      if (!func.locals.at(loc_idx).non_aggregate.arg_idx.has_value()) {
        func.locals.erase(loc_idx);
        removed++;
      }
    }
  }
  return removed;
}

static std::size_t NullifyUnusedVars(calyx::Function& function, FunctionDependencies& dependencies) {
  std::size_t removed = 0;
  cotyl::unordered_set<var_index_t> todo{};
  // copy map keys
  std::transform(dependencies.var_graph.begin(), dependencies.var_graph.end(), std::inserter(todo, todo.begin()),
                 [](auto& kv) { return kv.first; });

  while (!todo.empty()) {
    const auto var_idx = *todo.begin();
    todo.erase(todo.begin());

    // remove nullified reads
    auto& var = dependencies.var_graph.at(var_idx);
    var.reads.erase(
      std::remove_if(
        var.reads.begin(), var.reads.end(),
        [&](const auto& pos) { 
          return calyx::IsType<calyx::NoOp>(function.blocks.at(pos.first).at(pos.second)); 
        }
      ), 
      var.reads.end()
    );

    // NEVER erase call results
    if (var.reads.empty() && !var.is_call_result) {
      // remove from all dependencies
      for (auto other : var.deps) {
        // other var is now also possibly unused
        todo.insert(other);
      }

      // nullify write
      function.blocks.at(var.created.first).at(var.created.second).template emplace<calyx::NoOp>();
      removed++;
    }
  }
  return removed;
}

std::size_t RemoveUnused(calyx::Function& function) {
  std::size_t removed = 0;
  auto deps = FunctionDependencies::GetDependencies(function);

  // this also removed any uses from the stored vars
  removed += NullifyUnusedLocals(function, deps);
  removed += NullifyUnusedVars(function, deps);

  // noops will be removed in the next optimization step
  return removed;
}

}