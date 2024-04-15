#include "Example.h"


namespace epi::calyx {

void Example::Emit(const AnyDirective& dir) {
  dir.visit<void>([&](const auto& d) { Emit(d); });
}

void Example::Emit(const Program& program) {

}

template<typename To, typename From>
void Example::Emit(const Cast<To, From>& op) {

}

template<typename T>
void Example::Emit(const LoadLocal<T>& op) {

}

void Example::Emit(const LoadLocalAddr& op) {

}

template<typename T>
void Example::Emit(const StoreLocal<T>& op) {

}

template<typename T>
void Example::Emit(const LoadGlobal<T>& op) {

}

void Example::Emit(const LoadGlobalAddr& op) {

}

template<typename T>
void Example::Emit(const StoreGlobal<T>& op) {

}

template<typename T>
void Example::Emit(const LoadFromPointer<T>& op) {
  
}

template<typename T>
void Example::Emit(const StoreToPointer<T>& op) {
  
}

template<typename T>
void Example::Emit(const Call<T>& op) {

}

template<typename T>
void Example::Emit(const CallLabel<T>& op) {

}

template<typename T>
void Example::Emit(const Return<T>& op) {

}

template<typename T>
void Example::Emit(const Imm<T>& op) {

}

template<typename T>
void Example::Emit(const Unop<T>& op) {

}

template<typename T>
void Example::Emit(const Binop<T>& op) {

}

template<typename T>
void Example::Emit(const Shift<T>& op) {

}

template<typename T>
void Example::Emit(const Compare<T>& op) {

}

void Example::Emit(const UnconditionalBranch& op) {

}

template<typename T>
void Example::Emit(const BranchCompare<T>& op) {

}

void Example::Emit(const Select& op) {

}

template<typename T>
void Example::Emit(const AddToPointer<T>& op) {

}

}