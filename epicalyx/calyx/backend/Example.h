#pragma once

#include "calyx/CalyxFwd.h"


namespace epi::calyx {

struct Example {

  void Emit(const calyx::Program& program);

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