#pragma once

#include "calyx/Calyx.h"
#include "Scope.h"
#include "CString.h"
#include "Containers.h"

#include "swl/variant.hpp"
#include <optional>
#include <stack>


namespace epi::calyx {

struct Interpreter {
  using program_counter_t = program_pos_t;

  void InterpretGlobalInitializer(Global& dest, Function&& func);
  i32 Interpret(const calyx::Program& program);

  // globals as raw data
  cotyl::unordered_map<cotyl::CString, calyx::Global> globals{};

  using pointer_real_t = swl::variant<i64, LabelOffset>;
  using var_real_t = swl::variant<
    Scalar<i32>, Scalar<u32>, 
    Scalar<i64>, Scalar<u64>, 
    Scalar<float>, Scalar<double>, 
    calyx::Pointer
  >;
private:

  cotyl::vector<u8> stack{};
  cotyl::vector<pointer_real_t> pointer_values{};

  calyx::Pointer MakePointer(pointer_real_t value) {
    const auto idx = pointer_values.size();
    pointer_values.emplace_back(value);
    return calyx::Pointer{(i64)idx};
  }

  pointer_real_t ReadPointer(i64 idx) const {
    return pointer_values.at(idx);
  }

  // points to stack location of locals
  cotyl::MapScope<var_index_t, std::pair<i64, u64>> locals{};

  // IR variables
  cotyl::MapScope<var_index_t, var_real_t, true> vars{};

  program_counter_t pos{nullptr, {0, 0}};
  std::optional<cotyl::CString> called{};
  // link, return_to, args, var_args
  std::stack<std::tuple<program_counter_t, var_index_t, const calyx::ArgData*>> call_stack{};
  std::optional<i32> returned = {};

  void DumpVars() const;

  void EnterFunction(const calyx::Function* function);
  void LoadArg(const calyx::Local& loc);
  void Emit(const calyx::AnyDirective& dir);

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