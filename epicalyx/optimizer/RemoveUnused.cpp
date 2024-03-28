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
      program.blocks.at(local.pos_made.first)[local.pos_made.second] = nullptr;
      for (auto [pos, var_idx] : local.writes) {
        program.blocks.at(pos.first)[pos.second] = nullptr;

        if (var_idx) {
          // remove use case from stored var
          dependencies.var_graph.at(var_idx).other_uses.erase(pos);
        }
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
    const auto& var = dependencies.var_graph.at(var_idx);

    if (var.other_uses.empty() && var.read_for.empty()) {
      // remove from all dependencies
      for (auto other : var.deps) {
        dependencies.var_graph.at(other).read_for.erase(var_idx);

        // other var is now also possibly unused
        todo.insert(other);
      }

      // nullify write
      program.blocks.at(var.pos_made.first)[var.pos_made.second] = nullptr;
    }
  }
}

void RemoveUnused(calyx::Program& program) {
  ProgramDependencies dependencies;
  dependencies.EmitProgram(program);

  // this also removed any uses from the stored vars
  NullifyUnusedLocals(program, dependencies);
  NullifyUnusedVars(program, dependencies);

  // remove nullified directives
  for (auto& [block_idx, block] : program.blocks) {
    block.erase(
        std::remove(block.begin(), block.end(), nullptr),
        block.end()
    );
  }
}

}