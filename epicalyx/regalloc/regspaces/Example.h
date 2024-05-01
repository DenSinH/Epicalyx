#pragma once

#include "RegisterSpace.h"
#include "Containers.h"


namespace epi {

struct ExampleRegSpace final : RegisterSpace {
  
  enum class RegType : register_type_t {
    GPR, FPR, Stack
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

  void Emit(const calyx::AnyDirective& dir) final;
  void Emit(var_index_t loc_idx, const calyx::Local& loc) final;

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