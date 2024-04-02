#pragma once

#include "calyx/backend/Backend.h"
#include "Containers.h"
#include "cycle/Graph.h"

#include <vector>

namespace epi {

using namespace calyx;

struct ProgramDependencies final : calyx::Backend {

  static ProgramDependencies GetDependencies(const Program& program) {
    auto deps = ProgramDependencies();
    deps.EmitProgram(program);
    return deps;
  }

  Graph<block_label_t, const Program::block_t*> block_graph{};

  struct Var {
    program_pos_t created = {0, 0};
    // variables depend on at most 2 others through a binop
    cotyl::static_vector<var_index_t, 2> deps{};
    bool is_call_result = false;
    std::vector<program_pos_t> reads{};
    program_pos_t program_result = {0, 0};
  };

  cotyl::unordered_map<var_index_t, Var> var_graph{};

  struct Local {
    program_pos_t created = {0, 0};
    std::vector<program_pos_t> writes{};
    std::vector<program_pos_t> reads{};
    bool needs_address = false;
  };

  cotyl::unordered_map<var_index_t, Local> local_graph{};

  program_pos_t pos;

  void VisualizeVars(const std::string& filename);

  void EmitProgram(const Program& program) final;

  void Emit(const AllocateLocal& op) final;
  void Emit(const DeallocateLocal& op) final;
  void Emit(const LoadLocalAddr& op) final;
  void Emit(const LoadGlobalAddr& op) final;
  void Emit(const ArgMakeLocal& op) final;

  template<typename To, typename From>
  void EmitCast(const Cast<To, From>& op);
  template<typename T>
  void EmitLoadLocal(const LoadLocal<T>& op);
  template<typename T>
  void EmitStoreLocal(const StoreLocal<T>& op);
  template<typename T>
  void EmitLoadGlobal(const LoadGlobal<T>& op);
  template<typename T>
  void EmitStoreGlobal(const StoreGlobal<T>& op);
  template<typename T>
  void EmitLoadFromPointer(const LoadFromPointer<T>& op);
  template<typename T>
  void EmitStoreToPointer(const StoreToPointer<T>& op);
  template<typename T>
  void EmitCall(const Call<T>& op);
  template<typename T>
  void EmitCallLabel(const CallLabel<T>& op);
  template<typename T>
  void EmitReturn(const Return<T>& op);
  template<typename T>
  void EmitImm(const Imm<T>& op);
  template<typename T>
  void EmitUnop(const Unop<T>& op);
  template<typename T>
  void EmitBinop(const Binop<T>& op);
  template<typename T>
  void EmitBinopImm(const BinopImm<T>& op);
  template<typename T>
  void EmitShift(const Shift<T>& op);
  template<typename T>
  void EmitShiftImm(const ShiftImm<T>& op);
  template<typename T>
  void EmitCompare(const Compare<T>& op);
  template<typename T>
  void EmitCompareImm(const CompareImm<T>& op);
  template<typename T>
  void EmitBranchCompare(const BranchCompare<T>& op);
  template<typename T>
  void EmitBranchCompareImm(const BranchCompareImm<T>& op);
  template<typename T>
  void EmitAddToPointer(const AddToPointer<T>& op);

#include "calyx/backend/Methods.inl"

};

}