#pragma once


#include "Backend.h"

namespace epi::calyx {

struct Example : Backend {

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

#include "Methods.inl"

};

}