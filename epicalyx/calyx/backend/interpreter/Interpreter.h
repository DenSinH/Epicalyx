#pragma once

#include "calyx/backend/Backend.h"
#include "Scope.h"
#include "Containers.h"

#include <variant>
#include <optional>
#include <stack>


namespace epi::calyx {

struct Interpreter : Backend {
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
  cotyl::unordered_map<std::string, i64> globals{};
  std::vector<std::vector<u8>> global_data{{}};

  // points to stack location of locals
  cotyl::MapScope<var_index_t, std::pair<i64, u64>> locals{};

  // IR variables
  cotyl::MapScope<var_index_t, std::variant<i32, u32, i64, u64, float, double, calyx::Pointer>, true> vars{};

  program_counter_t pos{nullptr, {0, 0}};
  // link, return_to, args, var_args
  std::stack<std::tuple<program_counter_t, var_index_t, calyx::arg_list_t, calyx::arg_list_t>> call_stack{};
  std::optional<std::variant<i32, u32, i64, u64, float, double, calyx::Pointer>> returned = {};

  void EnterFunction(const calyx::Function* function);
  void LoadArg(const calyx::Local& loc);
  void EmitProgram(const calyx::Program& program);

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

#include "../Methods.inl"
};

}