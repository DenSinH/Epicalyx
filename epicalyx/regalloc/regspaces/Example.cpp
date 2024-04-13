#include "Example.h"
#include "CustomAssert.h"
#include "Format.h"

#include <type_traits>


namespace epi {

using namespace calyx;

register_type_t ExampleRegSpace::RegisterType(const GeneralizedVar& gvar) const {
  return static_cast<register_type_t>(register_type_map.at(gvar));
}

std::size_t ExampleRegSpace::RegisterTypePopulation(const register_type_t& type) const {
  switch (static_cast<RegType>(type)) {
    case RegType::GPR: return 16;
    case RegType::FPR: return 16;
    default: throw cotyl::FormatExcept("Invalid register type: %d", type);
  }
}

std::optional<register_t> ExampleRegSpace::ForcedRegister(const GeneralizedVar& gvar) const {
  return {};
}

template<typename T>
void ExampleRegSpace::OutputGVar(const GeneralizedVar& gvar) {
  RegType type;
  if constexpr(std::is_integral_v<T>) {
    type = RegType::GPR;
  }
  else if constexpr(std::is_same_v<T, calyx::Pointer>) {
    type = RegType::GPR;
  }
  else {
    type = RegType::FPR;
  }
  cotyl::Assert(!register_type_map.contains(gvar) || (register_type_map[gvar] == type));
  register_type_map[gvar] = type;
}

template<typename T>
void ExampleRegSpace::OutputVar(var_index_t var_idx) {
  OutputGVar<T>(GeneralizedVar::Var(var_idx));
}

template<typename T>
void ExampleRegSpace::OutputExpr(const Expr& expr) {
  static_assert(is_calyx_type_v<T>, "Invalid type for expression variable result");
  OutputVar<T>(expr.idx);
}

template<typename T>
void ExampleRegSpace::OutputLocal(var_index_t loc_idx) {
  const auto gvar = GeneralizedVar::Local(loc_idx);
  OutputGVar<T>(gvar);
}

template<typename To, typename From>
void ExampleRegSpace::Emit(const Cast<To, From>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputVar<calyx_op_type(op)::src_t>(op.right_idx);
}

template<typename T>
void ExampleRegSpace::Emit(const LoadLocal<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputLocal<T>(op.loc_idx);
}

void ExampleRegSpace::Emit(const LoadLocalAddr& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
}

template<typename T>
void ExampleRegSpace::Emit(const StoreLocal<T>& op) {
  OutputLocal<T>(op.loc_idx);
  if (op.src.IsVar()) OutputVar<calyx_op_type(op)::src_t>(op.src.GetVar());
}

template<typename T>
void ExampleRegSpace::Emit(const LoadGlobal<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
}

void ExampleRegSpace::Emit(const LoadGlobalAddr& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
}

template<typename T>
void ExampleRegSpace::Emit(const StoreGlobal<T>& op) {
  if (op.src.IsVar()) OutputVar<calyx_op_type(op)::src_t>(op.src.GetVar());
}

template<typename T>
void ExampleRegSpace::Emit(const LoadFromPointer<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputVar<calyx::Pointer>(op.ptr_idx);
}

template<typename T>
void ExampleRegSpace::Emit(const StoreToPointer<T>& op) {
  if (op.src.IsVar()) OutputVar<calyx_op_type(op)::src_t>(op.src.GetVar());
  OutputVar<calyx::Pointer>(op.ptr_idx);
}

template<typename T>
void ExampleRegSpace::Emit(const Call<T>& op) {
  if constexpr(!std::is_same_v<T, void>) {
    OutputVar<calyx_op_type(op)::result_t>(op.idx);
  }
  OutputVar<calyx::Pointer>(op.fn_idx);
}

template<typename T>
void ExampleRegSpace::Emit(const CallLabel<T>& op) {
  if constexpr(!std::is_same_v<T, void>) {
    OutputVar<calyx_op_type(op)::result_t>(op.idx);
  }
}

template<typename T>
void ExampleRegSpace::Emit(const Return<T>& op) {
  if constexpr(!std::is_same_v<T, void>) {
    if (op.val.IsVar()) OutputVar<calyx_op_type(op)::src_t>(op.val.GetVar());
  }
}

template<typename T>
void ExampleRegSpace::Emit(const Imm<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
}

template<typename T>
void ExampleRegSpace::Emit(const Unop<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputVar<calyx_op_type(op)::src_t>(op.right_idx);
}

template<typename T>
void ExampleRegSpace::Emit(const Binop<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputVar<calyx_op_type(op)::src_t>(op.left_idx);
  if (op.right.IsVar()) OutputVar<calyx_op_type(op)::src_t>(op.right.GetVar());
}

template<typename T>
void ExampleRegSpace::Emit(const Shift<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  if (op.left.IsVar()) OutputVar<calyx_op_type(op)::src_t>(op.left.GetVar());
  if (op.right.IsVar()) OutputVar<calyx_op_type(op)::shift_t>(op.right.GetVar());
}

template<typename T>
void ExampleRegSpace::Emit(const Compare<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputVar<calyx_op_type(op)::src_t>(op.left_idx);
  if (op.right.IsVar()) OutputVar<calyx_op_type(op)::src_t>(op.right.GetVar());
}

void ExampleRegSpace::Emit(const UnconditionalBranch& op) {

}

template<typename T>
void ExampleRegSpace::Emit(const BranchCompare<T>& op) {
  OutputVar<calyx_op_type(op)::src_t>(op.left_idx);
  if (op.right.IsVar()) OutputVar<calyx_op_type(op)::src_t>(op.right.GetVar());
}

void ExampleRegSpace::Emit(const Select& op) {
  OutputVar<calyx_op_type(op)::src_t>(op.idx);
}

template<typename T>
void ExampleRegSpace::Emit(const AddToPointer<T>& op) {
  OutputExpr<calyx::Pointer>(op);
  if (op.ptr.IsVar()) OutputVar<calyx::Pointer>(op.ptr.GetVar());
  if (op.right.IsVar()) OutputVar<calyx_op_type(op)::offset_t>(op.right.GetVar());
}

}