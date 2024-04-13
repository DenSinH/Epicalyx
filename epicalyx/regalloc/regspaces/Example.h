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
  void OutputExpr(const calyx::Expr& expr);
  template<typename T>
  void OutputLocal(var_index_t loc_idx);

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

}