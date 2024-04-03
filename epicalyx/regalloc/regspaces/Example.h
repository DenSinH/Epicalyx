#pragma once

#include "RegisterSpace.h"
#include "Containers.h"


namespace epi {

struct ExampleRegSpace final : RegisterSpace {
  
  enum class RegType : register_type_t {
    GPR, FPR
  };
  
  cotyl::unordered_map<GeneralizedVar, RegType> register_type_map{};

  register_type_t RegisterType(const GeneralizedVar& gvar) const final;
  std::size_t RegisterTypePopulation(const register_type_t& type) const final;
  std::optional<register_t> ForcedRegister(const GeneralizedVar& gvar) const final;

  template<typename T>
  void OutputGVar(const GeneralizedVar& gvar);
  template<typename T>
  void OutputVar(var_index_t var_idx);
  template<typename T>
  void OutputExpr(const Expr& expr);
  template<typename T>
  void OutputLocal(var_index_t loc_idx);

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