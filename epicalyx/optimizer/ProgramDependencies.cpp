#include "ProgramDependencies.h"

#include "Containers.h"
#include "cycle/Cycle.h"

#include <algorithm>


namespace epi {

namespace detail {

template<typename K, typename T>
T& get_default(cotyl::unordered_map<K, T>& graph, const K& key) {
  if (!graph.contains(key)) {
    graph[key] = {};
  }
  return graph[key];
}

//template<typename K, typename T>
//void add_default(cotyl::unordered_map<K, std::vector<T>>& graph, const K& key, const T& value) {
//  get_default(graph, key);
//  graph.at(key).push_back(value);
//}

}


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
    graph->n(idx, std::to_string(var.read_for.size() + var.other_uses) + " reads");
    for (const auto& dep : var.deps) {
      graph->n(idx)->n(dep);
    }
  }

  graph->Visualize();
  graph->Join();
}

void ProgramDependencies::EmitProgram(Program& program) {
  for (const auto& [i, block] : program.blocks) {
    detail::get_default(block_graph, i);
    pos.first = i;
    for (int j = 0; j < block.size(); j++) {
      pos.second = j;
      block[j]->Emit(*this);
    }
  }
}

void ProgramDependencies::Emit(AllocateLocal& op) {
  detail::get_default(local_graph, op.loc_idx).pos_made = pos;
}

void ProgramDependencies::Emit(DeallocateLocal& op) {
  // count this as a write to the local
  detail::get_default(local_graph, op.loc_idx).writes.insert({pos, 0});
}

template<typename To, typename From>
void ProgramDependencies::EmitCast(Cast<To, From>& op) {
  detail::get_default(var_graph, op.idx).pos_made = pos;
  var_graph.at(op.idx).deps.emplace(op.right_idx);
  detail::get_default(var_graph, op.right_idx).read_for.insert(op.idx);
}

template<typename T>
void ProgramDependencies::EmitLoadLocal(LoadLocal<T>& op) {
  detail::get_default(var_graph, op.idx).pos_made = pos;
  detail::get_default(local_graph, op.loc_idx).reads++;
}

void ProgramDependencies::Emit(LoadLocalAddr& op) {
  detail::get_default(var_graph, op.idx).pos_made = pos;
  detail::get_default(local_graph, op.loc_idx).reads++;
}

template<typename T>
void ProgramDependencies::EmitStoreLocal(StoreLocal<T>& op) {
  detail::get_default(var_graph, op.src).other_uses++;
  detail::get_default(local_graph, op.loc_idx).writes.insert({pos, op.src});
}

template<typename T>
void ProgramDependencies::EmitLoadGlobal(LoadGlobal<T>& op) {
  detail::get_default(var_graph, op.idx).pos_made = pos;
}

void ProgramDependencies::Emit(LoadGlobalAddr& op) {
  detail::get_default(var_graph, op.idx).pos_made = pos;
}

template<typename T>
void ProgramDependencies::EmitStoreGlobal(StoreGlobal<T>& op) {
  detail::get_default(var_graph, op.src).other_uses++;
}

template<typename T>
void ProgramDependencies::EmitLoadFromPointer(LoadFromPointer<T>& op) {
  detail::get_default(var_graph, op.idx).pos_made = pos;
  var_graph.at(op.idx).deps.emplace(op.ptr_idx);
  detail::get_default(var_graph, op.ptr_idx).read_for.insert(op.idx);
}

template<typename T>
void ProgramDependencies::EmitStoreToPointer(StoreToPointer<T>& op) {
  detail::get_default(var_graph, op.src).other_uses++;
  detail::get_default(var_graph, op.ptr_idx).other_uses++;
}

template<typename T>
void ProgramDependencies::EmitCall(Call<T>& op) {
  detail::get_default(var_graph, op.fn_idx).other_uses++;
  if constexpr(!std::is_same_v<T, void>) {
    detail::get_default(var_graph, op.idx).pos_made = pos;
  }

  for (const auto& [var_idx, arg] : op.args) {
    detail::get_default(var_graph, op.idx).deps.emplace(var_idx);
    detail::get_default(var_graph, var_idx).other_uses++;
  }
  for (const auto& [var_idx, arg] : op.var_args) {
    detail::get_default(var_graph, op.idx).deps.emplace(var_idx);
    detail::get_default(var_graph, var_idx).other_uses++;
  }
}

template<typename T>
void ProgramDependencies::EmitCallLabel(CallLabel<T>& op) {
  if constexpr(!std::is_same_v<T, void>) {
    detail::get_default(var_graph, op.idx).pos_made = pos;
  }
  for (const auto&[var_idx, arg] : op.args) {
    detail::get_default(var_graph, op.idx).deps.emplace(var_idx);
    detail::get_default(var_graph, var_idx).other_uses++;
  }
  for (const auto&[var_idx, arg] : op.var_args) {
    detail::get_default(var_graph, op.idx).deps.emplace(var_idx);
    detail::get_default(var_graph, var_idx).other_uses++;
  }
}

void ProgramDependencies::Emit(ArgMakeLocal& op) {
  detail::get_default(local_graph, op.loc_idx).pos_made = pos;
}

template<typename T>
void ProgramDependencies::EmitReturn(Return<T>& op) {
  detail::get_default(var_graph, op.idx).other_uses++;
}

template<typename T>
void ProgramDependencies::EmitImm(Imm<T>& op) {
  detail::get_default(var_graph, op.idx).pos_made = pos;
}

template<typename T>
void ProgramDependencies::EmitUnop(Unop<T>& op) {
  detail::get_default(var_graph, op.idx).pos_made = pos;
  var_graph.at(op.idx).deps.emplace(op.right_idx);
  detail::get_default(var_graph, op.right_idx).read_for.insert(op.idx);
}

template<typename T>
void ProgramDependencies::EmitBinop(Binop<T>& op) {
  detail::get_default(var_graph, op.idx).pos_made = pos;
  detail::get_default(var_graph, op.idx).deps.emplace(op.left_idx);
  detail::get_default(var_graph, op.left_idx).read_for.insert(op.idx);
  detail::get_default(var_graph, op.idx).deps.emplace(op.right_idx);
  detail::get_default(var_graph, op.right_idx).read_for.insert(op.idx);
}

template<typename T>
void ProgramDependencies::EmitBinopImm(BinopImm<T>& op) {
  detail::get_default(var_graph, op.idx).pos_made = pos;
  detail::get_default(var_graph, op.idx).deps.emplace(op.left_idx);
  detail::get_default(var_graph, op.left_idx).read_for.insert(op.idx);
}

template<typename T>
void ProgramDependencies::EmitShift(Shift<T>& op) {
  detail::get_default(var_graph, op.idx).pos_made = pos;
  detail::get_default(var_graph, op.idx).deps.emplace(op.left_idx);
  detail::get_default(var_graph, op.left_idx).read_for.insert(op.idx);
  detail::get_default(var_graph, op.idx).deps.emplace(op.right_idx);
  detail::get_default(var_graph, op.right_idx).read_for.insert(op.idx);
}

template<typename T>
void ProgramDependencies::EmitShiftImm(ShiftImm<T>& op) {
  detail::get_default(var_graph, op.idx).pos_made = pos;
  detail::get_default(var_graph, op.idx).deps.emplace(op.left_idx);
  detail::get_default(var_graph, op.left_idx).read_for.insert(op.idx);
}

template<typename T>
void ProgramDependencies::EmitCompare(Compare<T>& op) {
  detail::get_default(var_graph, op.idx).pos_made = pos;
  detail::get_default(var_graph, op.idx).deps.emplace(op.left_idx);
  detail::get_default(var_graph, op.left_idx).read_for.insert(op.idx);
  detail::get_default(var_graph, op.idx).deps.emplace(op.right_idx);
  detail::get_default(var_graph, op.right_idx).read_for.insert(op.idx);
}

template<typename T>
void ProgramDependencies::EmitCompareImm(CompareImm<T>& op) {
  detail::get_default(var_graph, op.idx).deps.emplace(op.left_idx);
  detail::get_default(var_graph, op.left_idx).read_for.insert(op.idx);
}

void ProgramDependencies::Emit(UnconditionalBranch& op) {
  detail::get_default(block_graph, pos.first).to.emplace(op.dest);
  detail::get_default(block_graph, op.dest).from.emplace(pos.first);
}

template<typename T>
void ProgramDependencies::EmitBranchCompare(BranchCompare<T>& op) {
  detail::get_default(block_graph, pos.first).to.emplace(op.dest);
  detail::get_default(block_graph, op.dest).from.emplace(pos.first);
  detail::get_default(var_graph, op.left_idx).other_uses++;
  detail::get_default(var_graph, op.right_idx).other_uses++;
}

template<typename T>
void ProgramDependencies::EmitBranchCompareImm(BranchCompareImm<T>& op) {
  detail::get_default(block_graph, pos.first).to.emplace(op.dest);
  detail::get_default(block_graph, op.dest).from.emplace(pos.first);
  detail::get_default(var_graph, op.left_idx).other_uses++;
}

void ProgramDependencies::Emit(Select& op) {
  detail::get_default(var_graph, op.idx).other_uses++;
  for (const auto& [value, block] : op.table) {
    detail::get_default(block_graph, pos.first).to.emplace(block);
    detail::get_default(block_graph, block).from.emplace(pos.first);
  }
  if (op._default) {
    detail::get_default(block_graph, pos.first).to.emplace(op._default);
    detail::get_default(block_graph, op._default).from.emplace(pos.first);
  }
}

template<typename T>
void ProgramDependencies::EmitAddToPointer(AddToPointer<T>& op) {
  detail::get_default(var_graph, op.idx).pos_made = pos;
  detail::get_default(var_graph, op.idx).deps.emplace(op.ptr_idx);
  detail::get_default(var_graph, op.ptr_idx).read_for.insert(op.idx);
  detail::get_default(var_graph, op.idx).deps.emplace(op.right_idx);
  detail::get_default(var_graph, op.right_idx).read_for.insert(op.idx);
}

void ProgramDependencies::Emit(AddToPointerImm& op) {
  detail::get_default(var_graph, op.idx).pos_made = pos;
  detail::get_default(var_graph, op.idx).deps.emplace(op.ptr_idx);
  detail::get_default(var_graph, op.ptr_idx).read_for.insert(op.idx);
}

#define BACKEND_NAME ProgramDependencies
#include "calyx/backend/Templates.inl"

}