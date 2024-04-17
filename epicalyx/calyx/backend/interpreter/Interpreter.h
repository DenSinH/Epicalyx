#pragma once

#include "calyx/Calyx.h"
#include "Scope.h"
#include "CString.h"
#include "Containers.h"

#include <variant>
#include <optional>
#include <stack>


namespace epi::calyx {

struct Interpreter {
  using program_counter_t = program_pos_t;

  Interpreter(const Program& program) : program(program) {

  }

  const Program& program;

  std::vector<u8> stack{};
  std::vector<std::variant<i64, label_offset_t>> pointer_values{};

  void InterpretGlobalInitializer(global_t& dest, Function&& func);

  calyx::Pointer MakePointer(std::variant<i64, label_offset_t> value) {
    const auto idx = pointer_values.size();
    pointer_values.emplace_back(value);
    return calyx::Pointer{(i64)idx};
  }

  std::variant<i64, label_offset_t> ReadPointer(i64 idx) const {
    return pointer_values.at(idx);
  }

  // globals as raw data
  cotyl::unordered_map<cotyl::CString, i64> globals{};
  std::vector<std::vector<u8>> global_data{{}};

  // points to stack location of locals
  cotyl::MapScope<var_index_t, std::pair<i64, u64>> locals{};

  // IR variables
  cotyl::MapScope<var_index_t, std::variant<i32, u32, i64, u64, float, double, calyx::Pointer>, true> vars{};

  program_counter_t pos{nullptr, {0, 0}};
  // link, return_to, args, var_args
  std::stack<std::tuple<program_counter_t, var_index_t, const calyx::ArgData*>> call_stack{};
  std::optional<std::variant<i32, u32, i64, u64, float, double, calyx::Pointer>> returned = {};

  void EnterFunction(const calyx::Function* function);
  void LoadArg(const calyx::Local& loc);
  void EmitProgram(const calyx::Program& program);
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

}