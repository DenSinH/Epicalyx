#pragma once

#include "calyx/backend/Backend.h"
#include "Containers.h"
#include "cycle/Graph.h"

#include <vector>
#include <optional>


namespace epi {


struct FunctionDependencies final : calyx::Backend {

  struct Var {
    func_pos_t created = {0, 0};
    // variables depend on at most 2 others through a binop
    cotyl::static_vector<var_index_t, 2> deps{};
    bool is_call_result = false;
    std::vector<func_pos_t> reads{};
    func_pos_t function_result = {0, 0};

    // local aliases (i.e. v12 = &c1)
    // 0 means no alias
    var_index_t aliases = 0;
  };

  struct LocalData {
    std::vector<func_pos_t> writes{};
    std::vector<func_pos_t> reads{};
    bool needs_address = false;

    std::vector<var_index_t> aliased_by{};
  };

  std::string symbol{};

  Graph<block_label_t, const calyx::block_t*, true> block_graph{};
  cotyl::unordered_map<var_index_t, Var> var_graph{};
  cotyl::unordered_map<var_index_t, LocalData> local_graph{};

  explicit FunctionDependencies() { }

  FunctionDependencies(const std::string& symbol) : symbol{symbol} { 

  }
  
  static FunctionDependencies GetDependencies(const calyx::Function& function) {
    auto deps = FunctionDependencies();
    deps.EmitFunction(function);
    return deps;
  }
  void EmitFunction(const calyx::Function& function);

protected:
  func_pos_t pos;

  void Emit(const calyx::LoadLocalAddr& op) final;
  void Emit(const calyx::LoadGlobalAddr& op) final;

  template<typename To, typename From>
  void EmitCast(const calyx::Cast<To, From>& op);
  template<typename T>
  void EmitLoadLocal(const calyx::LoadLocal<T>& op);
  template<typename T>
  void EmitStoreLocal(const calyx::StoreLocal<T>& op);
  template<typename T>
  void EmitLoadGlobal(const calyx::LoadGlobal<T>& op);
  template<typename T>
  void EmitStoreGlobal(const calyx::StoreGlobal<T>& op);
  template<typename T>
  void EmitLoadFromPointer(const calyx::LoadFromPointer<T>& op);
  template<typename T>
  void EmitStoreToPointer(const calyx::StoreToPointer<T>& op);
  template<typename T>
  void EmitCall(const calyx::Call<T>& op);
  template<typename T>
  void EmitCallLabel(const calyx::CallLabel<T>& op);
  template<typename T>
  void EmitReturn(const calyx::Return<T>& op);
  template<typename T>
  void EmitImm(const calyx::Imm<T>& op);
  template<typename T>
  void EmitUnop(const calyx::Unop<T>& op);
  template<typename T>
  void EmitBinop(const calyx::Binop<T>& op);
  template<typename T>
  void EmitShift(const calyx::Shift<T>& op);
  template<typename T>
  void EmitCompare(const calyx::Compare<T>& op);
  template<typename T>
  void EmitBranchCompare(const calyx::BranchCompare<T>& op);
  template<typename T>
  void EmitAddToPointer(const calyx::AddToPointer<T>& op);

#include "calyx/backend/Methods.inl"

};

struct ProgramDependencies {

  cotyl::unordered_map<std::string, FunctionDependencies> func_deps{};

  static ProgramDependencies GetDependencies(const calyx::Program& program) {
    auto deps = ProgramDependencies();
    deps.ParseProgram(program);
    return deps;
  }

  void VisualizeVars(const std::string& filename);

private:
  void ParseProgram(const calyx::Program& program);
};

}