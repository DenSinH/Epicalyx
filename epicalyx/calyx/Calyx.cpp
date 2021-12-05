#include "Calyx.h"
#include "backend/Backend.h"

#include "Format.h"


namespace epi::calyx {

namespace detail {

template<typename T>
struct type_string {
  static const std::string value;
};

template<> const std::string type_string<i8>::value = "i8";
template<> const std::string type_string<u8>::value = "u8";
template<> const std::string type_string<i16>::value = "i16";
template<> const std::string type_string<u16>::value = "u16";
template<> const std::string type_string<i32>::value = "i32";
template<> const std::string type_string<u32>::value = "u32";
template<> const std::string type_string<i64>::value = "i64";
template<> const std::string type_string<u64>::value = "u64";
template<> const std::string type_string<float>::value = "float";
template<> const std::string type_string<double>::value = "double";
template<> const std::string type_string<Struct>::value = "struct";
template<> const std::string type_string<Pointer>::value = "pointer";

}

template<typename To, typename From>
std::string Cast<To, From>::ToString() const {
  return cotyl::FormatStr(
      "cast  v%s <%s> <- <%s>v%s", this->idx, detail::type_string<To>::value, detail::type_string<From>::value, right_idx
  );
}

template<typename T>
requires (is_calyx_type_v<T>)
std::string Binop<T>::ToString() const {
  std::string op_str;
  switch (op) {
    case BinopType::Add: op_str = "+"; break;
    case BinopType::Sub: op_str = "-"; break;
    case BinopType::Mul: op_str = "*"; break;
    case BinopType::Div: op_str = "/"; break;
    case BinopType::Mod: op_str = "%"; break;
    case BinopType::BinAnd: op_str = "&"; break;
    case BinopType::BinOr: op_str = "|"; break;
    case BinopType::BinXor: op_str = "^"; break;
  }
  return cotyl::FormatStr("binop v%s = v%s %s<%s> v%s", this->idx, left_idx, op_str, detail::type_string<T>::value, right_idx);
}

template<typename T>
requires (is_calyx_type_v<T>)
std::string BinopImm<T>::ToString() const {
  std::string op_str;
  switch (op) {
    case BinopType::Add: op_str = "+"; break;
    case BinopType::Sub: op_str = "-"; break;
    case BinopType::Mul: op_str = "*"; break;
    case BinopType::Div: op_str = "/"; break;
    case BinopType::Mod: op_str = "%"; break;
    case BinopType::BinAnd: op_str = "&"; break;
    case BinopType::BinOr: op_str = "|"; break;
    case BinopType::BinXor: op_str = "^"; break;
  }
  return cotyl::FormatStr("binim v%s = v%s %s<%s> %s", this->idx, left_idx, op_str, detail::type_string<T>::value, right);
}

template<typename T>
requires (is_calyx_type_v<T>)
std::string Shift<T>::ToString() const {
  std::string op_str;
  switch (op) {
    case ShiftType::Left: op_str = "<<"; break;
    case ShiftType::Right: op_str = ">>"; break;
  }
  return cotyl::FormatStr("shift v%s = v%s %s<%s> v%s", this->idx, left_idx, op_str, detail::type_string<T>::value, right_idx);
}

template<typename T>
requires (is_calyx_type_v<T>)
std::string ShiftImm<T>::ToString() const {
  std::string op_str;
  switch (op) {
    case ShiftType::Left: op_str = "<<"; break;
    case ShiftType::Right: op_str = ">>"; break;
  }
  return cotyl::FormatStr("shift v%s = v%s %s<%s> %s", this->idx, left_idx, op_str, detail::type_string<T>::value, right);
}

static std::string cmp_string(CmpType type) {
  switch (type) {
    case CmpType::Eq: return "==";
    case CmpType::Ne: return "!=";
    case CmpType::Lt: return "<";
    case CmpType::Le: return "<=";
    case CmpType::Gt: return ">";
    case CmpType::Ge: return ">=";
  }
}

static std::string to_string(const Pointer& ptr) {
  return cotyl::Format("%016x", ptr.value);
}

template<typename T>
requires (is_calyx_type_v<T>)
std::string Compare<T>::ToString() const {
  return cotyl::FormatStr(
          "cmp   v%s = v%s %s<%s> v%s",
          this->idx, left_idx, cmp_string(op),
          detail::type_string<T>::value, right_idx
  );
}

template<typename T>
requires (is_calyx_type_v<T>)
std::string CompareImm<T>::ToString() const {
    return cotyl::FormatStr(
            "cmpim v%s = v%s %s<%s> %s",
            this->idx, left_idx, cmp_string(op),
            detail::type_string<T>::value, right
    );
}

std::string UnconditionalBranch::ToString() const {
  return cotyl::FormatStr("brnch L%s", this->dest);
}

template<typename T>
requires (is_calyx_type_v<T>)
std::string BranchCompare<T>::ToString() const {
  return cotyl::FormatStr(
          "brnch L%s : v%s %s<%s> v%s",
          this->dest, left_idx, cmp_string(op),
          detail::type_string<T>::value, right_idx
  );
}

template<typename T>
requires (is_calyx_type_v<T>)
std::string BranchCompareImm<T>::ToString() const {
  return cotyl::FormatStr(
          "brnch L%s : v%s %s<%s> imm(%s)",
          this->dest, left_idx, cmp_string(op),
          detail::type_string<T>::value, right
  );
}

std::string Select::ToString() const {
  return cotyl::FormatStr("select v%s", idx);
}

template<typename T>
requires (is_calyx_integral_type_v<T>)
std::string AddToPointer<T>::ToString() const {
  return cotyl::FormatStr("ptrad v%s = v%s + %s * v%s", this->idx, ptr_idx, stride, right_idx);
}

std::string AddToPointerImm::ToString() const {
  return cotyl::FormatStr("paddi v%s = v%s + %s * imm(%s)", this->idx, ptr_idx, stride, right);
}

template<typename T>
requires (is_calyx_type_v<T>)
std::string Imm<T>::ToString() const {
  return cotyl::FormatStr("immed v%s = imm<%s>(%s)", this->idx, detail::type_string<T>::value, value);
}

template<typename T>
requires (is_calyx_type_v<T>)
std::string Unop<T>::ToString() const {
  std::string op_str;
  switch (op) {
    case UnopType::Neg: op_str = "-"; break;
    case UnopType::BinNot: op_str = "~"; break;
  }
  return cotyl::FormatStr("unop  v%s = %s<%s> v%s", this->idx, op_str, detail::type_string<T>::value, right_idx);
}

std::string AllocateCVar::ToString() const {
  return cotyl::FormatStr("alloc c%s: %s", c_idx, size);
}

std::string DeallocateCVar::ToString() const {
  return cotyl::FormatStr("dallc c%s: %s", c_idx, size);
}

template<typename T>
std::string LoadCVar<T>::ToString() const {
  return cotyl::FormatStr("load  v%s <-<%s> c%s", this->idx, detail::type_string<T>::value, c_idx);
}

template<typename T>
std::string StoreCVar<T>::ToString() const {
  return cotyl::FormatStr("store c%s = v%s <-<%s> v%s", c_idx, this->idx, detail::type_string<T>::value, src);
}

std::string LoadCVarAddr::ToString() const {
  return cotyl::FormatStr("addrss v%s <- c%s", idx, c_idx);
}

template<typename T>
requires (is_calyx_type_v<T>)
std::string Return<T>::ToString() const {
  if (idx) {
    return cotyl::FormatStr("retrn [%s]v%s", detail::type_string<T>::value, idx);
  }
  return "retrn void";
}


template<typename To, typename From>
void Cast<To, From>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T>)
void Binop<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T>)
void BinopImm<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T>)
void Shift<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T>)
void ShiftImm<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T>)
void Compare<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T>)
void CompareImm<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

void UnconditionalBranch::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T>)
void BranchCompare<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T>)
void BranchCompareImm<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T>)
void Unop<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_integral_type_v<T>)
void AddToPointer<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

void AddToPointerImm::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T>)
void Imm<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
void LoadCVar<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

void LoadCVarAddr::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
void StoreCVar<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

void AllocateCVar::Emit(Backend& backend) {
  backend.Emit(*this);
}

void DeallocateCVar::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T>)
void LoadFromPointer<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T>)
void StoreToPointer<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T>)
void Return<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

void Select::Emit(Backend& backend) {
  backend.Emit(*this);
}

template struct Binop<i32>;
template struct Binop<u32>;
template struct Binop<i64>;
template struct Binop<u64>;
template struct Binop<float>;
template struct Binop<double>;

template struct BinopImm<i32>;
template struct BinopImm<u32>;
template struct BinopImm<i64>;
template struct BinopImm<u64>;
template struct BinopImm<float>;
template struct BinopImm<double>;

template struct Shift<i32>;
template struct Shift<u32>;
template struct Shift<i64>;
template struct Shift<u64>;
template struct ShiftImm<i32>;
template struct ShiftImm<u32>;
template struct ShiftImm<i64>;
template struct ShiftImm<u64>;

template struct Compare<i32>;
template struct Compare<u32>;
template struct Compare<i64>;
template struct Compare<u64>;
template struct Compare<float>;
template struct Compare<double>;
template struct Compare<Pointer>;

template struct CompareImm<i32>;
template struct CompareImm<u32>;
template struct CompareImm<i64>;
template struct CompareImm<u64>;
template struct CompareImm<float>;
template struct CompareImm<double>;
template struct CompareImm<Pointer>;

template struct BranchCompare<i32>;
template struct BranchCompare<u32>;
template struct BranchCompare<i64>;
template struct BranchCompare<u64>;
template struct BranchCompare<float>;
template struct BranchCompare<double>;
template struct BranchCompare<Pointer>;

template struct BranchCompareImm<i32>;
template struct BranchCompareImm<u32>;
template struct BranchCompareImm<i64>;
template struct BranchCompareImm<u64>;
template struct BranchCompareImm<float>;
template struct BranchCompareImm<double>;
template struct BranchCompareImm<Pointer>;

template struct Return<i32>;
template struct Return<u32>;
template struct Return<i64>;
template struct Return<u64>;
template struct Return<float>;
template struct Return<double>;
template struct Return<Pointer>;
template struct Return<Struct>;

template struct AddToPointer<i32>;
template struct AddToPointer<u32>;
template struct AddToPointer<i64>;
template struct AddToPointer<u64>;

template struct Unop<i32>;
template struct Unop<u32>;
template struct Unop<i64>;
template struct Unop<u64>;
template struct Unop<float>;
template struct Unop<double>;

template struct Imm<i32>;
template struct Imm<u32>;
template struct Imm<i64>;
template struct Imm<u64>;
template struct Imm<float>;
template struct Imm<double>;

template struct LoadCVar<i8>;
template struct LoadCVar<u8>;
template struct LoadCVar<i16>;
template struct LoadCVar<u16>;
template struct LoadCVar<i32>;
template struct LoadCVar<u32>;
template struct LoadCVar<i64>;
template struct LoadCVar<u64>;
template struct LoadCVar<float>;
template struct LoadCVar<double>;
template struct LoadCVar<Struct>;
template struct LoadCVar<Pointer>;

template struct StoreCVar<i8>;
template struct StoreCVar<u8>;
template struct StoreCVar<i16>;
template struct StoreCVar<u16>;
template struct StoreCVar<i32>;
template struct StoreCVar<u32>;
template struct StoreCVar<i64>;
template struct StoreCVar<u64>;
template struct StoreCVar<float>;
template struct StoreCVar<double>;
template struct StoreCVar<Struct>;
template struct StoreCVar<Pointer>;

template struct Cast<i8, i32>;
template struct Cast<i8, u32>;
template struct Cast<i8, i64>;
template struct Cast<i8, u64>;
template struct Cast<i8, float>;
template struct Cast<i8, double>;
template struct Cast<i8, Pointer>;
template struct Cast<i16, i32>;
template struct Cast<i16, u32>;
template struct Cast<i16, i64>;
template struct Cast<i16, u64>;
template struct Cast<i16, float>;
template struct Cast<i16, double>;
template struct Cast<i16, Pointer>;
template struct Cast<i32, i32>;
template struct Cast<i32, u32>;
template struct Cast<i32, i64>;
template struct Cast<i32, u64>;
template struct Cast<i32, float>;
template struct Cast<i32, double>;
template struct Cast<i32, Pointer>;
template struct Cast<i64, i32>;
template struct Cast<i64, u32>;
template struct Cast<i64, i64>;
template struct Cast<i64, u64>;
template struct Cast<i64, float>;
template struct Cast<i64, double>;
template struct Cast<i64, Pointer>;
template struct Cast<u8, i32>;
template struct Cast<u8, u32>;
template struct Cast<u8, i64>;
template struct Cast<u8, u64>;
template struct Cast<u8, float>;
template struct Cast<u8, double>;
template struct Cast<u8, Pointer>;
template struct Cast<u16, i32>;
template struct Cast<u16, u32>;
template struct Cast<u16, i64>;
template struct Cast<u16, u64>;
template struct Cast<u16, float>;
template struct Cast<u16, double>;
template struct Cast<u16, Pointer>;
template struct Cast<u32, i32>;
template struct Cast<u32, u32>;
template struct Cast<u32, i64>;
template struct Cast<u32, u64>;
template struct Cast<u32, float>;
template struct Cast<u32, double>;
template struct Cast<u32, Pointer>;
template struct Cast<u64, i32>;
template struct Cast<u64, u32>;
template struct Cast<u64, i64>;
template struct Cast<u64, u64>;
template struct Cast<u64, float>;
template struct Cast<u64, double>;
template struct Cast<u64, Pointer>;
template struct Cast<float, i32>;
template struct Cast<float, u32>;
template struct Cast<float, i64>;
template struct Cast<float, u64>;
template struct Cast<float, float>;
template struct Cast<float, double>;
template struct Cast<float, Pointer>;
template struct Cast<double, i32>;
template struct Cast<double, u32>;
template struct Cast<double, i64>;
template struct Cast<double, u64>;
template struct Cast<double, float>;
template struct Cast<double, double>;
template struct Cast<double, Pointer>;
template struct Cast<Pointer, i32>;
template struct Cast<Pointer, u32>;
template struct Cast<Pointer, i64>;
template struct Cast<Pointer, u64>;
template struct Cast<Pointer, float>;
template struct Cast<Pointer, double>;
template struct Cast<Pointer, Pointer>;

}