#include "ProgramDependencies.h"

#include "Containers.h"
#include "cycle/Cycle.h"
#include "SStream.h"
#include "Format.h"

#include <algorithm>


namespace epi {


calyx::block_label_t ProgramDependencies::CommonBlockAncestor(calyx::block_label_t first, calyx::block_label_t second) const {
  cotyl::set<calyx::block_label_t> ancestors{first, second};

  // we use the fact that in general block1 > block2 then block1 can never be an ancestor of block2
  // it may happen for loops, but then the loop entry is the minimum block, so we want to go there
  cotyl::unordered_set<calyx::block_label_t> ancestors_considered{};

  while (ancestors.size() > 1) {
    auto max_ancestor = *ancestors.rbegin();
    ancestors.erase(std::prev(ancestors.end()));
    ancestors_considered.emplace(max_ancestor);
    if (!block_graph.contains(max_ancestor)) [[unlikely]] {
      return 0;
    }

    auto& deps = block_graph.at(max_ancestor);
    if (deps.from.empty()) [[unlikely]] {
      return 0;
    }
    for (auto dep : deps.from) {
      if (dep < max_ancestor || !ancestors_considered.contains(dep)) {
        ancestors.insert(dep);
      }
    }
  }

  // at this point only one ancestor should be left
  return *ancestors.begin();
}

std::vector<calyx::block_label_t> ProgramDependencies::OrderedUpwardClosure(calyx::block_label_t base) const {
  cotyl::unordered_set<calyx::block_label_t> closure_found{base};
  std::vector<calyx::block_label_t> closure{};
  closure.push_back(base);
  cotyl::unordered_set<calyx::block_label_t> search{base};

  while (!search.empty()) {
    auto current = *search.begin();
    search.erase(search.begin());

    if (block_graph.contains(current)) {
      for (const auto& dep : block_graph.at(current).to) {
        if (!closure_found.contains(dep)) {
          closure_found.emplace(dep);
          search.emplace(dep);
          closure.push_back(dep);
        }
      }
    }
  }

  return closure;
}

cotyl::unordered_set<calyx::block_label_t> ProgramDependencies::UpwardClosure(cotyl::unordered_set<calyx::block_label_t>&& base) const {
  cotyl::unordered_set<calyx::block_label_t> closure = std::move(base);
  cotyl::unordered_set<calyx::block_label_t> search = {closure.begin(), closure.end()};

  while (!search.empty()) {
    auto current = *search.begin();
    search.erase(search.begin());

    if (block_graph.contains(current)) {
      for (const auto& dep : block_graph.at(current).to) {
        if (!closure.contains(dep)) {
          closure.emplace(dep);
          search.emplace(dep);
        }
      }
    }
  }

  return closure;
}

bool ProgramDependencies::IsAncestorOf(calyx::block_label_t base, calyx::block_label_t other) const {
  cotyl::unordered_set<calyx::block_label_t> closure_found{base};
  cotyl::unordered_set<calyx::block_label_t> search{base};

  while (!search.empty()) {
    auto current = *search.begin();
    search.erase(search.begin());

    if (block_graph.contains(current)) {
      for (const auto& dep : block_graph.at(current).to) {
        if (dep == other) {
          return true;
        }

        if (!closure_found.contains(dep)) {
          closure_found.emplace(dep);
          search.emplace(dep);
        }
      }
    }
  }

  return false;
}


void ProgramDependencies::VisualizeVars() {
  auto graph = std::make_unique<epi::cycle::Graph>();

  for (const auto& [idx, var] : var_graph) {
    if (!idx) continue;
    cotyl::StringStream text{
      cotyl::Format(
        "created [%d].[%d]\n%d reads", 
        var.created.first, var.created.second, var.reads.size()
      )
    };
    if (!var.reads.empty()) {
      text << ":\n";
      for (const auto& pos : var.reads) {
        text << cotyl::Format("read @[%d].[%d]\n", pos.first, pos.second);
      }
    }
    graph->n(idx, text.finalize());
    for (const auto& dep : var.deps) {
      graph->n(idx)->n(dep);
    }
  }

  graph->Visualize();
  graph->Join();
}

void ProgramDependencies::EmitProgram(const Program& program) {
  for (const auto& [i, block] : program.blocks) {
    cotyl::get_default(block_graph, i);
    pos.first = i;
    for (int j = 0; j < block.size(); j++) {
      pos.second = j;
      block[j]->Emit(*this);
    }
  }
}

void ProgramDependencies::Emit(const AllocateLocal& op) {
  cotyl::get_default(local_graph, op.loc_idx).created = pos;
}

void ProgramDependencies::Emit(const DeallocateLocal& op) {
  // count this as a write to the local
  cotyl::get_default(local_graph, op.loc_idx).writes.push_back(pos);
}

template<typename To, typename From>
void ProgramDependencies::EmitCast(const Cast<To, From>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  var_graph.at(op.idx).deps.push_back(op.right_idx);
  cotyl::get_default(var_graph, op.right_idx).reads.push_back(pos);
}

template<typename T>
void ProgramDependencies::EmitLoadLocal(const LoadLocal<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(local_graph, op.loc_idx).reads.push_back(pos);
}

void ProgramDependencies::Emit(const LoadLocalAddr& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(local_graph, op.loc_idx).reads.push_back(pos);
}

template<typename T>
void ProgramDependencies::EmitStoreLocal(const StoreLocal<T>& op) {
  cotyl::get_default(var_graph, op.src).reads.push_back(pos);
  cotyl::get_default(local_graph, op.loc_idx).writes.push_back(pos);
}

template<typename T>
void ProgramDependencies::EmitLoadGlobal(const LoadGlobal<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
}

void ProgramDependencies::Emit(const LoadGlobalAddr& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
}

template<typename T>
void ProgramDependencies::EmitStoreGlobal(const StoreGlobal<T>& op) {
  cotyl::get_default(var_graph, op.src).reads.push_back(pos);
}

template<typename T>
void ProgramDependencies::EmitLoadFromPointer(const LoadFromPointer<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  var_graph.at(op.idx).deps.push_back(op.ptr_idx);
  cotyl::get_default(var_graph, op.ptr_idx).reads.push_back(pos);
}

template<typename T>
void ProgramDependencies::EmitStoreToPointer(const StoreToPointer<T>& op) {
  cotyl::get_default(var_graph, op.src).reads.push_back(pos);
  cotyl::get_default(var_graph, op.ptr_idx).reads.push_back(pos);
}

template<typename T>
void ProgramDependencies::EmitCall(const Call<T>& op) {
  cotyl::get_default(var_graph, op.fn_idx).reads.push_back(pos);
  if constexpr(!std::is_same_v<T, void>) {
    cotyl::get_default(var_graph, op.idx).created = pos;
    var_graph.at(op.idx).is_call_result = true;
  }

  for (const auto& [var_idx, arg] : op.args) {
    cotyl::get_default(var_graph, var_idx).reads.push_back(pos);
  }
  for (const auto& [var_idx, arg] : op.var_args) {
    cotyl::get_default(var_graph, var_idx).reads.push_back(pos);
  }
}

template<typename T>
void ProgramDependencies::EmitCallLabel(const CallLabel<T>& op) {
  if constexpr(!std::is_same_v<T, void>) {
    cotyl::get_default(var_graph, op.idx).created = pos;
    var_graph.at(op.idx).is_call_result = true;
  }
  for (const auto& [var_idx, arg] : op.args) {
    cotyl::get_default(var_graph, var_idx).reads.push_back(pos);
  }
  for (const auto& [var_idx, arg] : op.var_args) {
    cotyl::get_default(var_graph, var_idx).reads.push_back(pos);
  }
}

void ProgramDependencies::Emit(const ArgMakeLocal& op) {
  cotyl::get_default(local_graph, op.loc_idx).created = pos;
}

template<typename T>
void ProgramDependencies::EmitReturn(const Return<T>& op) {
  cotyl::get_default(var_graph, op.idx).reads.push_back(pos);
}

template<typename T>
void ProgramDependencies::EmitImm(const Imm<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
}

template<typename T>
void ProgramDependencies::EmitUnop(const Unop<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  var_graph.at(op.idx).deps.push_back(op.right_idx);
  cotyl::get_default(var_graph, op.right_idx).reads.push_back(pos);
}

template<typename T>
void ProgramDependencies::EmitBinop(const Binop<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.left_idx);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.right_idx);
  cotyl::get_default(var_graph, op.right_idx).reads.push_back(pos);
}

template<typename T>
void ProgramDependencies::EmitBinopImm(const BinopImm<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.left_idx);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
}

template<typename T>
void ProgramDependencies::EmitShift(const Shift<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.left_idx);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.right_idx);
  cotyl::get_default(var_graph, op.right_idx).reads.push_back(pos);
}

template<typename T>
void ProgramDependencies::EmitShiftImm(const ShiftImm<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.left_idx);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
}

template<typename T>
void ProgramDependencies::EmitCompare(const Compare<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.left_idx);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.right_idx);
  cotyl::get_default(var_graph, op.right_idx).reads.push_back(pos);
}

template<typename T>
void ProgramDependencies::EmitCompareImm(const CompareImm<T>& op) {
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.left_idx);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
}

void ProgramDependencies::Emit(const UnconditionalBranch& op) {
  cotyl::get_default(block_graph, pos.first).to.emplace(op.dest);
  cotyl::get_default(block_graph, op.dest).from.emplace(pos.first);
}

template<typename T>
void ProgramDependencies::EmitBranchCompare(const BranchCompare<T>& op) {
  cotyl::get_default(block_graph, pos.first).to.emplace(op.dest);
  cotyl::get_default(block_graph, op.dest).from.emplace(pos.first);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
  cotyl::get_default(var_graph, op.right_idx).reads.push_back(pos);
}

template<typename T>
void ProgramDependencies::EmitBranchCompareImm(const BranchCompareImm<T>& op) {
  cotyl::get_default(block_graph, pos.first).to.emplace(op.dest);
  cotyl::get_default(block_graph, op.dest).from.emplace(pos.first);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
}

void ProgramDependencies::Emit(const Select& op) {
  cotyl::get_default(var_graph, op.idx).reads.push_back(pos);
  for (const auto& [value, block] : op.table) {
    cotyl::get_default(block_graph, pos.first).to.emplace(block);
    cotyl::get_default(block_graph, block).from.emplace(pos.first);
  }
  if (op._default) {
    cotyl::get_default(block_graph, pos.first).to.emplace(op._default);
    cotyl::get_default(block_graph, op._default).from.emplace(pos.first);
  }
}

template<typename T>
void ProgramDependencies::EmitAddToPointer(const AddToPointer<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.ptr_idx);
  cotyl::get_default(var_graph, op.ptr_idx).reads.push_back(pos);
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.right_idx);
  cotyl::get_default(var_graph, op.right_idx).reads.push_back(pos);
}

void ProgramDependencies::Emit(const AddToPointerImm& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.ptr_idx);
  cotyl::get_default(var_graph, op.ptr_idx).reads.push_back(pos);
}

#define BACKEND_NAME ProgramDependencies
#include "calyx/backend/Templates.inl"

}