#pragma once

#include "calyx/backend/Backend.h"
#include "Scope.h"

#include <variant>
#include <optional>
#include <unordered_map>
#include <stack>


namespace epi::calyx {

struct Interpreter : Backend {
  using program_counter_t = std::pair<block_label_t, int>;

  Interpreter(const Program& program) : program(program) {

  }

  const Program& program;

  std::vector<u8> stack{};
  std::vector<std::variant<i64, label_offset_t>> pointer_values{};

  void InterpretGlobalInitializer(Program::global_t& dest, block_label_t entry);

  calyx::Pointer MakePointer(std::variant<i64, label_offset_t> value) {
    const auto idx = pointer_values.size();
    pointer_values.emplace_back(value);
    return calyx::Pointer{(i64)idx};
  }

  std::variant<i64, label_offset_t> ReadPointer(i64 idx) const {
    return pointer_values.at(idx);
  }

  // globals as raw data
  std::unordered_map<std::string, i64> globals{};
  std::vector<std::vector<u8>> global_data{{}};

  // points to stack location of locals
  cotyl::MapScope<calyx::var_index_t, std::pair<i64, u64>> locals{};

  // IR variables
  cotyl::MapScope<calyx::var_index_t, std::variant<i32, u32, i64, u64, float, double, calyx::Pointer>, true> vars{};

  program_counter_t pos{0, 0};
  // link, return_to, args, var_args
  std::stack<std::tuple<program_counter_t, calyx::var_index_t, calyx::arg_list_t, calyx::arg_list_t>> call_stack{};
  std::optional<std::variant<i32, u32, i64, u64, float, double, calyx::Pointer>> returned = {};

  void EmitProgram(Program& program) final;
  void VisualizeProgram(const Program& program);

  void Emit(AllocateLocal& op) final;
  void Emit(DeallocateLocal& op) final;
  void Emit(LoadLocalAddr& op) final;
  void Emit(LoadGlobalAddr& op) final;
  void Emit(ArgMakeLocal& op) final;

  template<typename To, typename From>
  void EmitCast(Cast<To, From>& op);
  template<typename T>
  void EmitLoadLocal(LoadLocal<T>& op);
  template<typename T>
  void EmitStoreLocal(StoreLocal<T>& op);
  template<typename T>
  void EmitLoadGlobal(LoadGlobal<T>& op);
  template<typename T>
  void EmitStoreGlobal(StoreGlobal<T>& op);
  template<typename T>
  void EmitLoadFromPointer(LoadFromPointer<T>& op);
  template<typename T>
  void EmitStoreToPointer(StoreToPointer<T>& op);
  template<typename T>
  void EmitCall(Call<T>& op);
  template<typename T>
  void EmitCallLabel(CallLabel<T>& op);
  template<typename T>
  void EmitReturn(Return<T>& op);
  template<typename T>
  void EmitImm(Imm<T>& op);
  template<typename T>
  void EmitUnop(Unop<T>& op);
  template<typename T>
  void EmitBinop(Binop<T>& op);
  template<typename T>
  void EmitBinopImm(BinopImm<T>& op);
  template<typename T>
  void EmitBranchCompare(BranchCompare<T>& op);
  template<typename T>
  void EmitBranchCompareImm(BranchCompareImm<T>& op);
  template<typename T>
  void EmitShift(Shift<T>& op);
  template<typename T>
  void EmitShiftImm(ShiftImm<T>& op);
  template<typename T>
  void EmitCompare(Compare<T>& op);
  template<typename T>
  void EmitCompareImm(CompareImm<T>& op);
  template<typename T>
  void EmitAddToPointer(AddToPointer<T>& op);

#include "../Methods.inl"
};

}