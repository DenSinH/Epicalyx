#pragma once

#include "calyx/backend/Backend.h"

#include <variant>
#include <unordered_map>


namespace epi::calyx {

struct Interpreter : Backend {

  std::vector<u8> stack{};

  // globals as raw data
  std::unordered_map<std::string, std::vector<u8>> globals{};

  // points to stack location of locals
  std::unordered_map<calyx::var_index_t, u64> locals{};

  // IR variables
  std::unordered_map<calyx::var_index_t, std::variant<i32, u32, i64, u64, float, double, calyx::Pointer>> vars{};

  std::pair<calyx::var_index_t, calyx::var_index_t> pos{0, 0};
  bool returned = false;

  void EmitProgram(Program& program) final;
  void VisualizeProgram(const Program& program);

  void Emit(AllocateLocal& op) final;
  void Emit(DeallocateLocal& op) final;
  void Emit(LoadLocalAddr& op) final;
  void Emit(LoadGlobalAddr& op) final;

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