#include "ProgramDependencies.h"

#include "Containers.h"
#include "cycle/Cycle.h"
#include "SStream.h"
#include "Format.h"

#include <algorithm>


namespace epi {


void ProgramDependencies::VisualizeVars() {
  auto graph = std::make_unique<epi::cycle::VisualGraph>();

  for (const auto& [idx, var] : var_graph) {
    if (!idx) continue;
    cotyl::StringStream text{
      cotyl::Format(
        "created [%d].[%d]\n%d reads\n", 
        var.created.first, var.created.second, var.reads.size()
      )
    };
    if (var.is_call_result) {
      text << "(CALL RESULT)\n";
    }
    if (!var.reads.empty()) {
      for (const auto& pos : var.reads) {
        text << cotyl::Format("read @[%d].[%d]\n", pos.first, pos.second);
      }
    }
    graph->n(idx, text.finalize());
    for (const auto& dep : var.deps) {
      graph->n(idx)->n(dep);
    }
  }

  graph->Visualize(cycle::VisualGraph::NodeSort::Topological);
  graph->Join();
}

void ProgramDependencies::EmitProgram(const Program& program) {
  // initialize block graph nodes
  block_graph.Reserve(program.blocks.size());
  for (const auto& [block_idx, block] : program.blocks) {
    block_graph.AddNode(block_idx, &block);
  }

  for (const auto& [i, block] : program.blocks) {
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
  var_graph.at(op.idx).program_result = pos;
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
  block_graph.AddEdge(pos.first, op.dest);
}

template<typename T>
void ProgramDependencies::EmitBranchCompare(const BranchCompare<T>& op) {
  block_graph.AddEdge(pos.first, op.dest);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
  cotyl::get_default(var_graph, op.right_idx).reads.push_back(pos);
}

template<typename T>
void ProgramDependencies::EmitBranchCompareImm(const BranchCompareImm<T>& op) {
  block_graph.AddEdge(pos.first, op.dest);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
}

void ProgramDependencies::Emit(const Select& op) {
  cotyl::get_default(var_graph, op.idx).reads.push_back(pos);
  for (const auto& [value, block_idx] : op.table) {
    block_graph.AddEdge(pos.first, block_idx);
  }
  if (op._default) {
    block_graph.AddEdge(pos.first, op._default);
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