#include "RemoveUnused.h"
#include "calyx/Calyx.h"
#include "ProgramDependencies.h"

#include <algorithm>


namespace epi {

static void NullifyUnusedLocals(calyx::Program& program, ProgramDependencies& dependencies) {
  // remove unused locals
  for (const auto& [loc_idx, local] : dependencies.local_graph) {
    if (local.reads.empty()) {
      // local is never read/aliased
      // remove creation and all local writes
      program.blocks.at(local.created.first)[local.created.second] = nullptr;
      for (const auto& pos : local.writes) {
        program.blocks.at(pos.first)[pos.second] = nullptr;
      }
    }
  }
}

static void NullifyUnusedVars(calyx::Program& program, ProgramDependencies& dependencies) {
  cotyl::unordered_set<var_index_t> todo{};
  // copy map keys
  std::transform(dependencies.var_graph.begin(), dependencies.var_graph.end(), std::inserter(todo, todo.begin()),
                 [](auto& kv) { return kv.first; });

  while (!todo.empty()) {
    const auto var_idx = *todo.begin();
    todo.erase(todo.begin());

    // remove nullified reads
    auto& var = dependencies.var_graph.at(var_idx);
    std::erase_if(var.reads, [&](const auto& pos) { return program.blocks.at(pos.first).at(pos.second) == nullptr; });

    // NEVER erase call results
    if (var.reads.empty() && !var.is_call_result) {
      // remove from all dependencies
      for (auto other : var.deps) {
        // other var is now also possibly unused
        todo.insert(other);
      }

      // nullify write
      program.blocks.at(var.created.first)[var.created.second] = nullptr;
    }
  }
}

size_t RemoveUnused(calyx::Program& program) {
  ProgramDependencies dependencies;
  dependencies.EmitProgram(program);

  // this also removed any uses from the stored vars
  NullifyUnusedLocals(program, dependencies);
  NullifyUnusedVars(program, dependencies);

  // remove nullified directives
  size_t removed_directives = 0;
  for (auto& [block_idx, block] : program.blocks) {
    removed_directives += std::erase(block, nullptr);
  }
  return removed_directives;
}

}