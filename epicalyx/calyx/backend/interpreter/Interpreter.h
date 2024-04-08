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
  cotyl::MapScope<calyx::var_index_t, std::pair<i64, u64>> locals{};

  // IR variables
  cotyl::MapScope<calyx::var_index_t, std::variant<i32, u32, i64, u64, float, double, calyx::Pointer>, true> vars{};

  program_counter_t pos{nullptr, {0, 0}};
  // link, return_to, args, var_args
  std::stack<std::tuple<program_counter_t, calyx::var_index_t, calyx::arg_list_t, calyx::arg_list_t>> call_stack{};
  std::optional<std::variant<i32, u32, i64, u64, float, double, calyx::Pointer>> returned = {};

  void EnterFunction(const Function* function);
  void LoadArg(const calyx::Local& loc);
  void EmitProgram(const Program& program);

  void Emit(const LoadLocalAddr& op) final;
  void Emit(const LoadGlobalAddr& op) final;

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
  void EmitBranchCompare(const BranchCompare<T>& op);
  template<typename T>
  void EmitBranchCompareImm(const BranchCompareImm<T>& op);
  template<typename T>
  void EmitShift(const Shift<T>& op);
  template<typename T>
  void EmitShiftImm(const ShiftImm<T>& op);
  template<typename T>
  void EmitCompare(const Compare<T>& op);
  template<typename T>
  void EmitCompareImm(const CompareImm<T>& op);
  template<typename T>
  void EmitAddToPointer(const AddToPointer<T>& op);

#include "../Methods.inl"
};

}