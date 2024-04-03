#include "Calyx.h"
#include "SStream.h"
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
requires (
  is_calyx_arithmetic_ptr_type_v<From> && 
  (is_calyx_arithmetic_ptr_type_v<To> || is_calyx_small_type_v<To>)
)
std::string Cast<To, From>::ToString() const {
  return cotyl::FormatStr(
      "cast  v%s <%s> <- <%s>v%s", this->idx, detail::type_string<To>::value, detail::type_string<From>::value, right_idx
  );
}

template<typename T>
requires (is_calyx_arithmetic_type_v<T>)
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
requires (is_calyx_arithmetic_type_v<T>)
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
requires (is_calyx_integral_type_v<T>)
std::string Shift<T>::ToString() const {
  std::string op_str;
  switch (op) {
    case ShiftType::Left: op_str = "<<"; break;
    case ShiftType::Right: op_str = ">>"; break;
  }
  return cotyl::FormatStr("shift v%s = v%s %s<%s> v%s", this->idx, left_idx, op_str, detail::type_string<T>::value, right_idx);
}

template<typename T>
requires (is_calyx_integral_type_v<T>)
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
  return cotyl::FormatStr("selct v%s", idx);
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
requires (is_calyx_arithmetic_type_v<T>)
std::string Unop<T>::ToString() const {
  std::string op_str;
  switch (op) {
    case UnopType::Neg: op_str = "-"; break;
    case UnopType::BinNot: op_str = "~"; break;
  }
  return cotyl::FormatStr("unop  v%s = %s<%s> v%s", this->idx, op_str, detail::type_string<T>::value, right_idx);
}

std::string AllocateLocal::ToString() const {
  return cotyl::FormatStr("alloc c%s: %s", loc_idx, size);
}

std::string DeallocateLocal::ToString() const {
  return cotyl::FormatStr("dallc c%s: %s", loc_idx, size);
}

template<typename T>
std::string LoadLocal<T>::ToString() const {
  return cotyl::FormatStr("load  v%s <-<%s> c%s", this->idx, detail::type_string<T>::value, loc_idx);
}

std::string LoadLocalAddr::ToString() const {
  return cotyl::FormatStr("addrs v%s <- c%s", idx, loc_idx);
}

template<typename T>
std::string StoreLocal<T>::ToString() const {
  return cotyl::FormatStr("store c%s = <%s> v%s", loc_idx, detail::type_string<T>::value, src);
}

template<typename T>
std::string LoadGlobal<T>::ToString() const {
  return cotyl::FormatStr("lglob v%s <-<%s> [%s]", this->idx, detail::type_string<T>::value, symbol);
}

std::string LoadGlobalAddr::ToString() const {
  return cotyl::FormatStr("adglb v%s <- &[%s]", idx, symbol);
}

template<typename T>
std::string StoreGlobal<T>::ToString() const {
  return cotyl::FormatStr("sglob [%s] = <%s> v%s", symbol, detail::type_string<T>::value, src);
}

template<typename T>
std::string LoadFromPointer<T>::ToString() const {
  return cotyl::FormatStr("deref v%s <-<%s> *v%s", this->idx, detail::type_string<T>::value, ptr_idx);
}

template<typename T>
std::string StoreToPointer<T>::ToString() const {
  return cotyl::FormatStr("store *v%s <-<%s> v%s", ptr_idx, detail::type_string<T>::value, src);
}

template<typename T>
requires (is_calyx_type_v<T> || std::is_same_v<T, void>)
std::string Return<T>::ToString() const {
  if constexpr(std::is_same_v<T, void>) {
    return "retrn void";
  }
  else {
    return cotyl::FormatStr("retrn [%s]v%s", detail::type_string<T>::value, idx);
  }
}

std::string make_args_list(const arg_list_t& args) {
  cotyl::StringStream stream{};
  if (!args.empty()) {
    for (int i = 0; i < args.size() - 1; i++) {
      stream << "v" << std::to_string(args[i].first) << ", ";
    }
    stream << "v" << std::to_string(args.back().first);
  }
  return stream.finalize();
}

template<typename T>
requires (is_calyx_type_v<T> || std::is_same_v<T, void>)
std::string Call<T>::ToString() const {

  if constexpr(std::is_same_v<T, void>) {
    if (!var_args.empty()) {
      return cotyl::FormatStr("call  [void]v%s(%s, ... %s)", fn_idx, make_args_list(args), make_args_list(var_args));
    }
    else {
      return cotyl::FormatStr("call  [void]v%s(%s)", fn_idx, make_args_list(args));
    }
  }
  else {
    if (!var_args.empty()) {
      return cotyl::FormatStr("call  v%s <- [%s]v%s(%s, ... %s)",
                              idx, detail::type_string<T>::value, fn_idx, make_args_list(args), make_args_list(var_args)
      );
    }
    else {
      return cotyl::FormatStr("call  v%s <- [%s]v%s(%s)", idx, detail::type_string<T>::value, fn_idx, make_args_list(args));
    }
  }
}

template<typename T>
requires (is_calyx_type_v<T> || std::is_same_v<T, void>)
std::string CallLabel<T>::ToString() const {

  if constexpr(std::is_same_v<T, void>) {
    if (!var_args.empty()) {
      return cotyl::FormatStr("call  [void]%s(%s, ... %s)", label, make_args_list(args), make_args_list(var_args));
    }
    else {
      return cotyl::FormatStr("call  [void]%s(%s)", label, make_args_list(args));
    }
  }
  else {
    if (!var_args.empty()) {
      return cotyl::FormatStr("call  v%s <- [%s]%s(%s, ... %s)",
                              idx, detail::type_string<T>::value, label, make_args_list(args), make_args_list(var_args)
      );
    }
    else {
      return cotyl::FormatStr("call  v%s <- [%s]%s(%s)", idx, detail::type_string<T>::value, label, make_args_list(args));
    }
  }
}

std::string ArgMakeLocal::ToString() const {
  return cotyl::FormatStr("mkloc c%s <- a%s", loc_idx, arg.arg_idx);
}

template<typename To, typename From>
requires (
  is_calyx_arithmetic_ptr_type_v<From> && 
  (is_calyx_arithmetic_ptr_type_v<To> || is_calyx_small_type_v<To>)
)
void Cast<To, From>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_arithmetic_type_v<T>)
void Binop<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_arithmetic_type_v<T>)
void BinopImm<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_integral_type_v<T>)
void Shift<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_integral_type_v<T>)
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
requires (is_calyx_arithmetic_type_v<T>)
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
void LoadLocal<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

void LoadLocalAddr::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
void StoreLocal<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
void LoadGlobal<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

void LoadGlobalAddr::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
void StoreGlobal<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

void AllocateLocal::Emit(Backend& backend) {
  backend.Emit(*this);
}

void DeallocateLocal::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
void LoadFromPointer<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
void StoreToPointer<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T> || std::is_same_v<T, void>)
void Call<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T> || std::is_same_v<T, void>)
void CallLabel<T>::Emit(Backend& backend) {
  backend.Emit(*this);
}

void ArgMakeLocal::Emit(Backend& backend) {
  backend.Emit(*this);
}

template<typename T>
requires (is_calyx_type_v<T> || std::is_same_v<T, void>)
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

template struct Call<i32>;
template struct Call<u32>;
template struct Call<i64>;
template struct Call<u64>;
template struct Call<float>;
template struct Call<double>;
template struct Call<Pointer>;
template struct Call<Struct>;
template struct Call<void>;

template struct CallLabel<i32>;
template struct CallLabel<u32>;
template struct CallLabel<i64>;
template struct CallLabel<u64>;
template struct CallLabel<float>;
template struct CallLabel<double>;
template struct CallLabel<Pointer>;
template struct CallLabel<Struct>;
template struct CallLabel<void>;

template struct Return<i32>;
template struct Return<u32>;
template struct Return<i64>;
template struct Return<u64>;
template struct Return<float>;
template struct Return<double>;
template struct Return<Pointer>;
template struct Return<Struct>;
template struct Return<void>;

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

template struct LoadLocal<i8>;
template struct LoadLocal<u8>;
template struct LoadLocal<i16>;
template struct LoadLocal<u16>;
template struct LoadLocal<i32>;
template struct LoadLocal<u32>;
template struct LoadLocal<i64>;
template struct LoadLocal<u64>;
template struct LoadLocal<float>;
template struct LoadLocal<double>;
template struct LoadLocal<Struct>;
template struct LoadLocal<Pointer>;

template struct StoreLocal<i8>;
template struct StoreLocal<u8>;
template struct StoreLocal<i16>;
template struct StoreLocal<u16>;
template struct StoreLocal<i32>;
template struct StoreLocal<u32>;
template struct StoreLocal<i64>;
template struct StoreLocal<u64>;
template struct StoreLocal<float>;
template struct StoreLocal<double>;
template struct StoreLocal<Struct>;
template struct StoreLocal<Pointer>;

template struct LoadGlobal<i8>;
template struct LoadGlobal<u8>;
template struct LoadGlobal<i16>;
template struct LoadGlobal<u16>;
template struct LoadGlobal<i32>;
template struct LoadGlobal<u32>;
template struct LoadGlobal<i64>;
template struct LoadGlobal<u64>;
template struct LoadGlobal<float>;
template struct LoadGlobal<double>;
template struct LoadGlobal<Struct>;
template struct LoadGlobal<Pointer>;

template struct StoreGlobal<i8>;
template struct StoreGlobal<u8>;
template struct StoreGlobal<i16>;
template struct StoreGlobal<u16>;
template struct StoreGlobal<i32>;
template struct StoreGlobal<u32>;
template struct StoreGlobal<i64>;
template struct StoreGlobal<u64>;
template struct StoreGlobal<float>;
template struct StoreGlobal<double>;
template struct StoreGlobal<Struct>;
template struct StoreGlobal<Pointer>;

template struct LoadFromPointer<i8>;
template struct LoadFromPointer<u8>;
template struct LoadFromPointer<i16>;
template struct LoadFromPointer<u16>;
template struct LoadFromPointer<i32>;
template struct LoadFromPointer<u32>;
template struct LoadFromPointer<i64>;
template struct LoadFromPointer<u64>;
template struct LoadFromPointer<float>;
template struct LoadFromPointer<double>;
template struct LoadFromPointer<Struct>;
template struct LoadFromPointer<Pointer>;

template struct StoreToPointer<i8>;
template struct StoreToPointer<u8>;
template struct StoreToPointer<i16>;
template struct StoreToPointer<u16>;
template struct StoreToPointer<i32>;
template struct StoreToPointer<u32>;
template struct StoreToPointer<i64>;
template struct StoreToPointer<u64>;
template struct StoreToPointer<float>;
template struct StoreToPointer<double>;
template struct StoreToPointer<Struct>;
template struct StoreToPointer<Pointer>;

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