#include "ProgramDependencies.h"

#include "cycle/Cycle.h"


namespace epi {

namespace detail {

template<typename K, typename T>
T& get_default(std::unordered_map<K, T>& graph, const K& key) {
  if (!graph.contains(key)) {
    graph[key] = {};
  }
  return graph[key];
}

//template<typename K, typename T>
//void add_default(std::unordered_map<K, std::vector<T>>& graph, const K& key, const T& value) {
//  get_default(graph, key);
//  graph.at(key).push_back(value);
//}

}

void ProgramDependencies::VisualizeVars() {
  auto graph = std::make_unique<epi::cycle::Graph>();

  for (const auto& [idx, var] : var_graph) {
    graph->n(idx, std::to_string(var.read_count) + " reads");
    for (const auto& dep : var.deps) {
      graph->n(idx)->n(dep);
    }
  }

  graph->Visualize();
  graph->Join();
}

void ProgramDependencies::EmitProgram(Program& program) {
  for (const auto& [i, block] : program.blocks) {
    pos.first = i;
    for (int j = 0; j < block.size(); j++) {
      pos.second = j;
      block[j]->Emit(*this);
    }
  }
}

void ProgramDependencies::Emit(AllocateLocal& op) {

}

void ProgramDependencies::Emit(DeallocateLocal& op) {

}

template<typename To, typename From>
void ProgramDependencies::EmitCast(Cast<To, From>& op) {
  detail::get_default(var_graph, op.idx).block_made = pos.first;
  var_graph.at(op.idx).deps.emplace(op.right_idx);
  detail::get_default(var_graph, op.right_idx).read_count++;
}

template<typename T>
void ProgramDependencies::EmitLoadLocal(LoadLocal<T>& op) {

}

void ProgramDependencies::Emit(LoadLocalAddr& op) {

}

template<typename T>
void ProgramDependencies::EmitStoreLocal(StoreLocal<T>& op) {
  detail::get_default(var_graph, op.src).read_count++;
}

template<typename T>
void ProgramDependencies::EmitLoadGlobal(LoadGlobal<T>& op) {

}

void ProgramDependencies::Emit(LoadGlobalAddr& op) {

}

template<typename T>
void ProgramDependencies::EmitStoreGlobal(StoreGlobal<T>& op) {
  detail::get_default(var_graph, op.idx).block_made = pos.first;
  var_graph.at(op.idx).deps.emplace(op.src);
  detail::get_default(var_graph, op.src).read_count++;
}

template<typename T>
void ProgramDependencies::EmitLoadFromPointer(LoadFromPointer<T>& op) {
  detail::get_default(var_graph, op.ptr_idx).read_count++;
}

template<typename T>
void ProgramDependencies::EmitStoreToPointer(StoreToPointer<T>& op) {
  detail::get_default(var_graph, op.src).read_count++;
  detail::get_default(var_graph, op.ptr_idx).read_count++;
}

template<typename T>
void ProgramDependencies::EmitCall(Call<T>& op) {
  if constexpr(!std::is_same_v<T, void>) {
    detail::get_default(var_graph, op.idx).block_made = pos.first;
    detail::get_default(var_graph, op.fn_idx).read_count++;
    for (const auto& [var_idx, arg] : op.args) {
      detail::get_default(var_graph, op.idx).deps.emplace(var_idx);
      detail::get_default(var_graph, var_idx).read_count++;
    }
    for (const auto& [var_idx, arg] : op.var_args) {
      detail::get_default(var_graph, op.idx).deps.emplace(var_idx);
      detail::get_default(var_graph, var_idx).read_count++;
    }
  }
}

template<typename T>
void ProgramDependencies::EmitCallLabel(CallLabel<T>& op) {
  if constexpr(!std::is_same_v<T, void>) {
    detail::get_default(var_graph, op.idx).block_made = pos.first;
    for (const auto&[var_idx, arg] : op.args) {
      detail::get_default(var_graph, op.idx).deps.emplace(var_idx);
      detail::get_default(var_graph, var_idx).read_count++;
    }
    for (const auto&[var_idx, arg] : op.var_args) {
      detail::get_default(var_graph, op.idx).deps.emplace(var_idx);
      detail::get_default(var_graph, var_idx).read_count++;
    }
  }
}

void ProgramDependencies::Emit(ArgMakeLocal& op) {

}

template<typename T>
void ProgramDependencies::EmitReturn(Return<T>& op) {
  detail::get_default(var_graph, op.idx).read_count++;
}

template<typename T>
void ProgramDependencies::EmitImm(Imm<T>& op) {
  detail::get_default(var_graph, op.idx).block_made = pos.first;
}

template<typename T>
void ProgramDependencies::EmitUnop(Unop<T>& op) {
  detail::get_default(var_graph, op.idx).block_made = pos.first;
  var_graph.at(op.idx).deps.emplace(op.right_idx);
  detail::get_default(var_graph, op.right_idx).read_count++;
}

template<typename T>
void ProgramDependencies::EmitBinop(Binop<T>& op) {
  detail::get_default(var_graph, op.idx).block_made = pos.first;
  detail::get_default(var_graph, op.idx).deps.emplace(op.left_idx);
  detail::get_default(var_graph, op.left_idx).read_count++;
  detail::get_default(var_graph, op.idx).deps.emplace(op.right_idx);
  detail::get_default(var_graph, op.right_idx).read_count++;
}

template<typename T>
void ProgramDependencies::EmitBinopImm(BinopImm<T>& op) {
  detail::get_default(var_graph, op.idx).block_made = pos.first;
  detail::get_default(var_graph, op.idx).deps.emplace(op.left_idx);
  detail::get_default(var_graph, op.left_idx).read_count++;
}

template<typename T>
void ProgramDependencies::EmitShift(Shift<T>& op) {
  detail::get_default(var_graph, op.idx).block_made = pos.first;
  detail::get_default(var_graph, op.idx).deps.emplace(op.left_idx);
  detail::get_default(var_graph, op.left_idx).read_count++;
  detail::get_default(var_graph, op.idx).deps.emplace(op.right_idx);
  detail::get_default(var_graph, op.right_idx).read_count++;
}

template<typename T>
void ProgramDependencies::EmitShiftImm(ShiftImm<T>& op) {
  detail::get_default(var_graph, op.idx).block_made = pos.first;
  detail::get_default(var_graph, op.idx).deps.emplace(op.left_idx);
  detail::get_default(var_graph, op.left_idx).read_count++;
}

template<typename T>
void ProgramDependencies::EmitCompare(Compare<T>& op) {
  detail::get_default(var_graph, op.idx).block_made = pos.first;
  detail::get_default(var_graph, op.idx).deps.emplace(op.left_idx);
  detail::get_default(var_graph, op.left_idx).read_count++;
  detail::get_default(var_graph, op.idx).deps.emplace(op.right_idx);
  detail::get_default(var_graph, op.right_idx).read_count++;
}

template<typename T>
void ProgramDependencies::EmitCompareImm(CompareImm<T>& op) {
  detail::get_default(var_graph, op.idx).deps.emplace(op.left_idx);
  detail::get_default(var_graph, op.left_idx).read_count++;
}

void ProgramDependencies::Emit(UnconditionalBranch& op) {
  detail::get_default(block_graph, pos.first).to.emplace(op.dest);
  detail::get_default(block_graph, op.dest).from.emplace(pos.first);
}

template<typename T>
void ProgramDependencies::EmitBranchCompare(BranchCompare<T>& op) {
  detail::get_default(block_graph, pos.first).to.emplace(op.dest);
  detail::get_default(block_graph, op.dest).from.emplace(pos.first);
  detail::get_default(var_graph, op.left_idx).read_count++;
  detail::get_default(var_graph, op.right_idx).read_count++;
}

template<typename T>
void ProgramDependencies::EmitBranchCompareImm(BranchCompareImm<T>& op) {
  detail::get_default(block_graph, pos.first).to.emplace(op.dest);
  detail::get_default(block_graph, op.dest).from.emplace(pos.first);
  detail::get_default(var_graph, op.left_idx).read_count++;
}

void ProgramDependencies::Emit(Select& op) {
  detail::get_default(var_graph, op.idx).read_count++;
  for (const auto& [value, block] : op.table) {
    detail::get_default(block_graph, pos.first).to.emplace(block);
    detail::get_default(block_graph, block).from.emplace(pos.first);
  }
}

template<typename T>
void ProgramDependencies::EmitAddToPointer(AddToPointer<T>& op) {
  detail::get_default(var_graph, op.idx).block_made = pos.first;
  detail::get_default(var_graph, op.idx).deps.emplace(op.ptr_idx);
  detail::get_default(var_graph, op.ptr_idx).read_count++;
  detail::get_default(var_graph, op.idx).deps.emplace(op.right_idx);
  detail::get_default(var_graph, op.right_idx).read_count++;
}

void ProgramDependencies::Emit(AddToPointerImm& op) {
  detail::get_default(var_graph, op.idx).block_made = pos.first;
  detail::get_default(var_graph, op.idx).deps.emplace(op.ptr_idx);
  detail::get_default(var_graph, op.ptr_idx).read_count++;
}

#define BACKEND_NAME ProgramDependencies
#include "calyx/backend/Templates.inl"

}