#include "ProgramDependencies.h"

#include "Containers.h"
#include "cycle/Cycle.h"
#include "SStream.h"
#include "Format.h"

#include <algorithm>


namespace epi {


void ProgramDependencies::VisualizeVars(const std::string& filename) {
  auto graph = std::make_unique<epi::cycle::VisualGraph>();

  for (const auto& [symbol, deps] : func_deps) {
    for (const auto& [idx, var] : deps.var_graph) {
      graph->n(idx, cotyl::Format(
          "created [%d].[%d]", var.created.first, var.created.second
        )
      )->title(cotyl::Format("v%d", idx));
      graph->n(idx, cotyl::Format("%d reads", var.reads.size()));
      if (var.is_call_result) {
        graph->n(idx, "(CALL RESULT)");
      }
      if (!var.reads.empty()) {
        for (const auto& pos : var.reads) {
          graph->n(idx, cotyl::Format("read @[%d].[%d]", pos.first, pos.second));
        }
      }
      
      for (const auto& dep : var.deps) {
        graph->n(dep)->n(idx);
      }
    }
  }

  graph->Visualize(filename);
}

void ProgramDependencies::ParseProgram(const Program& program) {
  for (const auto& [symbol, func] : program.functions) {
    func_deps.emplace(symbol, FunctionDependencies::GetDependencies(func));
  }
}

void FunctionDependencies::EmitFunction(const Function& function) { 
  // initialize block graph nodes
  block_graph.Reserve(function.blocks.size());
  for (const auto& [block_idx, block] : function.blocks) {
    block_graph.AddNodeIfNotExists(block_idx, &block);
  }

  local_graph.reserve(function.locals.size());
  for (const auto& [loc_idx, local] : function.locals) {
    local_graph.emplace(loc_idx, LocalData{});
  }

  for (const auto& [i, block] : function.blocks) {
    pos.first = i;
    for (int j = 0; j < block.size(); j++) {
      pos.second = j;
      block[j]->Emit(*this);
    }
  }
}

template<typename To, typename From>
void FunctionDependencies::EmitCast(const Cast<To, From>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  var_graph.at(op.idx).deps.push_back(op.right_idx);
  cotyl::get_default(var_graph, op.right_idx).reads.push_back(pos);
}

template<typename T>
void FunctionDependencies::EmitLoadLocal(const LoadLocal<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  local_graph.at(op.loc_idx).reads.push_back(pos);
}

void FunctionDependencies::Emit(const LoadLocalAddr& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  var_graph[op.idx].aliases = op.loc_idx;
  local_graph.at(op.loc_idx).reads.push_back(pos);
  local_graph.at(op.loc_idx).needs_address = true;
  local_graph.at(op.loc_idx).aliased_by.emplace_back(op.idx);
}

template<typename T>
void FunctionDependencies::EmitStoreLocal(const StoreLocal<T>& op) {
  cotyl::get_default(var_graph, op.src).reads.push_back(pos);
  local_graph.at(op.loc_idx).writes.push_back(pos);
}

template<typename T>
void FunctionDependencies::EmitLoadGlobal(const LoadGlobal<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
}

void FunctionDependencies::Emit(const LoadGlobalAddr& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
}

template<typename T>
void FunctionDependencies::EmitStoreGlobal(const StoreGlobal<T>& op) {
  cotyl::get_default(var_graph, op.src).reads.push_back(pos);
}

template<typename T>
void FunctionDependencies::EmitLoadFromPointer(const LoadFromPointer<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  var_graph.at(op.idx).deps.push_back(op.ptr_idx);
  cotyl::get_default(var_graph, op.ptr_idx).reads.push_back(pos);
}

template<typename T>
void FunctionDependencies::EmitStoreToPointer(const StoreToPointer<T>& op) {
  cotyl::get_default(var_graph, op.src).reads.push_back(pos);
  cotyl::get_default(var_graph, op.ptr_idx).reads.push_back(pos);
}

template<typename T>
void FunctionDependencies::EmitCall(const Call<T>& op) {
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
void FunctionDependencies::EmitCallLabel(const CallLabel<T>& op) {
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

void FunctionDependencies::Emit(const ArgMakeLocal& op) {
  local_graph.at(op.loc_idx).writes.push_back(pos);
}

template<typename T>
void FunctionDependencies::EmitReturn(const Return<T>& op) {
  if constexpr(!std::is_same_v<T, void>) {
    cotyl::get_default(var_graph, op.idx).reads.push_back(pos);
    var_graph.at(op.idx).function_result = pos;
  }
}

template<typename T>
void FunctionDependencies::EmitImm(const Imm<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
}

template<typename T>
void FunctionDependencies::EmitUnop(const Unop<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  var_graph.at(op.idx).deps.push_back(op.right_idx);
  cotyl::get_default(var_graph, op.right_idx).reads.push_back(pos);
}

template<typename T>
void FunctionDependencies::EmitBinop(const Binop<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.left_idx);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.right_idx);
  cotyl::get_default(var_graph, op.right_idx).reads.push_back(pos);
}

template<typename T>
void FunctionDependencies::EmitBinopImm(const BinopImm<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.left_idx);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
}

template<typename T>
void FunctionDependencies::EmitShift(const Shift<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.left_idx);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.right_idx);
  cotyl::get_default(var_graph, op.right_idx).reads.push_back(pos);
}

template<typename T>
void FunctionDependencies::EmitShiftImm(const ShiftImm<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.left_idx);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
}

template<typename T>
void FunctionDependencies::EmitCompare(const Compare<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.left_idx);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.right_idx);
  cotyl::get_default(var_graph, op.right_idx).reads.push_back(pos);
}

template<typename T>
void FunctionDependencies::EmitCompareImm(const CompareImm<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.left_idx);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
}

void FunctionDependencies::Emit(const UnconditionalBranch& op) {
  block_graph.AddEdge(pos.first, op.dest);
}

template<typename T>
void FunctionDependencies::EmitBranchCompare(const BranchCompare<T>& op) {
  block_graph.AddEdge(pos.first, op.tdest);
  block_graph.AddEdge(pos.first, op.fdest);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
  cotyl::get_default(var_graph, op.right_idx).reads.push_back(pos);
}

template<typename T>
void FunctionDependencies::EmitBranchCompareImm(const BranchCompareImm<T>& op) {
  block_graph.AddEdge(pos.first, op.tdest);
  block_graph.AddEdge(pos.first, op.fdest);
  cotyl::get_default(var_graph, op.left_idx).reads.push_back(pos);
}

void FunctionDependencies::Emit(const Select& op) {
  cotyl::get_default(var_graph, op.idx).reads.push_back(pos);
  for (const auto& [value, block_idx] : op.table) {
    block_graph.AddEdge(pos.first, block_idx);
  }
  if (op._default) {
    block_graph.AddEdge(pos.first, op._default);
  }
}

template<typename T>
void FunctionDependencies::EmitAddToPointer(const AddToPointer<T>& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.ptr_idx);
  cotyl::get_default(var_graph, op.ptr_idx).reads.push_back(pos);
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.right_idx);
  cotyl::get_default(var_graph, op.right_idx).reads.push_back(pos);
}

void FunctionDependencies::Emit(const AddToPointerImm& op) {
  cotyl::get_default(var_graph, op.idx).created = pos;
  cotyl::get_default(var_graph, op.idx).deps.push_back(op.ptr_idx);
  cotyl::get_default(var_graph, op.ptr_idx).reads.push_back(pos);
}

#define BACKEND_NAME FunctionDependencies
#include "calyx/backend/Templates.inl"

}