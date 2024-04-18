#pragma once

#include "calyx/Calyx.h"
#include "Containers.h"
#include "cycle/Graph.h"

#include <vector>
#include <optional>


namespace epi {


struct FunctionDependencies {

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

  cotyl::CString symbol{};

  Graph<block_label_t, const calyx::BasicBlock*, true> block_graph{};
  cotyl::unordered_map<var_index_t, Var> var_graph{};
  cotyl::unordered_map<var_index_t, LocalData> local_graph{};

  explicit FunctionDependencies() { }
  FunctionDependencies(cotyl::CString&& symbol) : symbol{std::move(symbol)} { }
  
  static FunctionDependencies GetDependencies(const calyx::Function& function) {
    auto deps = FunctionDependencies();
    deps.EmitFunction(function);
    return deps;
  }
  void EmitFunction(const calyx::Function& function);

protected:
  func_pos_t pos;

  void Emit(const calyx::AnyDirective& dir);

private:
  void Emit(const calyx::NoOp& op) { }
  template<typename To, typename From>
  void Emit(const calyx::Cast<To, From>& op);
  template<typename T>
  void Emit(const calyx::LoadLocal<T>& op);
  void Emit(const calyx::LoadLocalAddr& op);
  template<typename T>
  void Emit(const calyx::StoreLocal<T>& op);
  template<typename T>
  void Emit(const calyx::LoadGlobal<T>& op);
  void Emit(const calyx::LoadGlobalAddr& op);
  template<typename T>
  void Emit(const calyx::StoreGlobal<T>& op);
  template<typename T>
  void Emit(const calyx::LoadFromPointer<T>& op);
  template<typename T>
  void Emit(const calyx::StoreToPointer<T>& op);
  template<typename T>
  void Emit(const calyx::AddToPointer<T>& op);
  template<typename T>
  void Emit(const calyx::Call<T>& op);
  template<typename T>
  void Emit(const calyx::CallLabel<T>& op);
  template<typename T>
  void Emit(const calyx::Return<T>& op);
  template<typename T>
  void Emit(const calyx::Imm<T>& op);
  template<typename T>
  void Emit(const calyx::Unop<T>& op);
  template<typename T>
  void Emit(const calyx::Binop<T>& op);
  template<typename T>
  void Emit(const calyx::Shift<T>& op);
  template<typename T>
  void Emit(const calyx::Compare<T>& op);
  template<typename T>
  void Emit(const calyx::BranchCompare<T>& op);
  void Emit(const calyx::UnconditionalBranch& op);
  void Emit(const calyx::Select& op);
};

struct ProgramDependencies {

  cotyl::unordered_map<cotyl::CString, FunctionDependencies> func_deps{};

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