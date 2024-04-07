#include "Example.h"
#include "CustomAssert.h"
#include "Format.h"

#include <type_traits>


namespace epi {

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
  cotyl::Assert(is_calyx_type_v<T>, "Invalid type for expression variable result");
  OutputVar<T>(expr.idx);
}

template<typename T>
void ExampleRegSpace::OutputLocal(var_index_t loc_idx) {
  const auto gvar = GeneralizedVar::Local(loc_idx);
  OutputGVar<T>(gvar);
}

void ExampleRegSpace::Emit(const AllocateLocal& op) { }
void ExampleRegSpace::Emit(const DeallocateLocal& op) { }

template<typename To, typename From>
void ExampleRegSpace::EmitCast(const Cast<To, From>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputVar<calyx_op_type(op)::src_t>(op.right_idx);
}

template<typename T>
void ExampleRegSpace::EmitLoadLocal(const LoadLocal<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputLocal<T>(op.loc_idx);
}

void ExampleRegSpace::Emit(const LoadLocalAddr& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
}

template<typename T>
void ExampleRegSpace::EmitStoreLocal(const StoreLocal<T>& op) {
  OutputLocal<T>(op.loc_idx);
  OutputVar<calyx_op_type(op)::src_t>(op.src);
}

template<typename T>
void ExampleRegSpace::EmitLoadGlobal(const LoadGlobal<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
}

void ExampleRegSpace::Emit(const LoadGlobalAddr& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
}

template<typename T>
void ExampleRegSpace::EmitStoreGlobal(const StoreGlobal<T>& op) {
  OutputVar<calyx_op_type(op)::src_t>(op.src);
}

template<typename T>
void ExampleRegSpace::EmitLoadFromPointer(const LoadFromPointer<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputVar<calyx::Pointer>(op.ptr_idx);
}

template<typename T>
void ExampleRegSpace::EmitStoreToPointer(const StoreToPointer<T>& op) {
  OutputVar<calyx_op_type(op)::src_t>(op.src);
  OutputVar<calyx::Pointer>(op.ptr_idx);
}

template<typename T>
void ExampleRegSpace::EmitCall(const Call<T>& op) {
  if constexpr(!std::is_same_v<T, void>) {
    OutputVar<calyx_op_type(op)::result_t>(op.idx);
  }
  OutputVar<calyx::Pointer>(op.fn_idx);
}

template<typename T>
void ExampleRegSpace::EmitCallLabel(const CallLabel<T>& op) {
  if constexpr(!std::is_same_v<T, void>) {
    OutputVar<calyx_op_type(op)::result_t>(op.idx);
  }
}

void ExampleRegSpace::Emit(const ArgMakeLocal& op) {
  switch (op.arg.type) {
    case Local::Type::I8: OutputLocal<i8>(op.loc_idx); break;
    case Local::Type::U8: OutputLocal<u8>(op.loc_idx); break;
    case Local::Type::I16: OutputLocal<u16>(op.loc_idx); break;
    case Local::Type::U16: OutputLocal<i16>(op.loc_idx); break;
    case Local::Type::I32: OutputLocal<i32>(op.loc_idx); break;
    case Local::Type::U32: OutputLocal<u32>(op.loc_idx); break;
    case Local::Type::I64: OutputLocal<i64>(op.loc_idx); break;
    case Local::Type::U64: OutputLocal<u64>(op.loc_idx); break;
    case Local::Type::Float: OutputLocal<float>(op.loc_idx); break;
    case Local::Type::Double: OutputLocal<double>(op.loc_idx); break;
    case Local::Type::Pointer: OutputLocal<calyx::Pointer>(op.loc_idx); break;
    case Local::Type::Struct: OutputLocal<calyx::Struct>(op.loc_idx); break;
  }
}

template<typename T>
void ExampleRegSpace::EmitReturn(const Return<T>& op) {
  if constexpr(!std::is_same_v<T, void>) {
    OutputVar<calyx_op_type(op)::src_t>(op.idx);
  }
}

template<typename T>
void ExampleRegSpace::EmitImm(const Imm<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
}

template<typename T>
void ExampleRegSpace::EmitUnop(const Unop<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputVar<calyx_op_type(op)::src_t>(op.right_idx);
}

template<typename T>
void ExampleRegSpace::EmitBinop(const Binop<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputVar<calyx_op_type(op)::src_t>(op.left_idx);
  OutputVar<calyx_op_type(op)::src_t>(op.right_idx);
}

template<typename T>
void ExampleRegSpace::EmitBinopImm(const BinopImm<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputVar<calyx_op_type(op)::src_t>(op.left_idx);
}

template<typename T>
void ExampleRegSpace::EmitShift(const Shift<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputVar<calyx_op_type(op)::src_t>(op.left_idx);
  OutputVar<calyx_op_type(op)::shift_t>(op.right_idx);
}

template<typename T>
void ExampleRegSpace::EmitShiftImm(const ShiftImm<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputVar<calyx_op_type(op)::src_t>(op.left_idx);
}

template<typename T>
void ExampleRegSpace::EmitCompare(const Compare<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputVar<calyx_op_type(op)::src_t>(op.left_idx);
  OutputVar<calyx_op_type(op)::src_t>(op.right_idx);
}

template<typename T>
void ExampleRegSpace::EmitCompareImm(const CompareImm<T>& op) {
  OutputExpr<calyx_op_type(op)::result_t>(op);
  OutputVar<calyx_op_type(op)::src_t>(op.left_idx);
}

void ExampleRegSpace::Emit(const UnconditionalBranch& op) {

}

template<typename T>
void ExampleRegSpace::EmitBranchCompare(const BranchCompare<T>& op) {
  OutputVar<calyx_op_type(op)::src_t>(op.left_idx);
  OutputVar<calyx_op_type(op)::src_t>(op.right_idx);
}

template<typename T>
void ExampleRegSpace::EmitBranchCompareImm(const BranchCompareImm<T>& op) {
  OutputVar<calyx_op_type(op)::src_t>(op.left_idx);
}

void ExampleRegSpace::Emit(const Select& op) {
  OutputVar<calyx_op_type(op)::src_t>(op.idx);
}

template<typename T>
void ExampleRegSpace::EmitAddToPointer(const AddToPointer<T>& op) {
  OutputExpr<calyx::Pointer>(op);
  OutputVar<calyx::Pointer>(op.ptr_idx);
  OutputVar<calyx_op_type(op)::offset_t>(op.right_idx);
}

void ExampleRegSpace::Emit(const AddToPointerImm& op) {
  OutputExpr<calyx::Pointer>(op);
  OutputVar<calyx::Pointer>(op.ptr_idx);
}

#define BACKEND_NAME ExampleRegSpace
#include "calyx/backend/Templates.inl"

}