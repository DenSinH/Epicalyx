#include "Calyx.h"

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

}

template<typename T>
std::string IRBinop<T>::ToString() const {
  std::string op_str;
  switch (op) {
    case Binop::Add: op_str = "+"; break;
    case Binop::Sub: op_str = "-"; break;
    case Binop::Mul: op_str = "*"; break;
    case Binop::Div: op_str = "/"; break;
    case Binop::Mod: op_str = "%"; break;
    case Binop::Lsl: op_str = "<<"; break;
    case Binop::Lsr: op_str = "l>>"; break;
    case Binop::Asr: op_str = "a>>"; break;
    case Binop::BinAnd: op_str = "&"; break;
    case Binop::BinOr: op_str = "|"; break;
    case Binop::BinXor: op_str = "^"; break;
  }
  return cotyl::FormatStr("assgn v%s = v%s %s<%s> v%s", idx, left_idx, op_str, detail::type_string<T>::value, right_idx);
}

template<typename T>
std::string IRImm<T>::ToString() const {
  return cotyl::FormatStr("immed v%s = imm<%s>(%s)", idx, detail::type_string<T>::value, value);
}

template<typename T>
std::string IRUnop<T>::ToString() const {
  std::string op_str;
  switch (op) {
    case Unop::Neg: op_str = "-"; break;
    case Unop::BinNot: op_str = "~"; break;
  }
  return cotyl::FormatStr("unop  v%s = %s<%s> v%s", idx, op_str, detail::type_string<T>::value, right_idx);
}

std::string IRAllocateCVar::ToString() const {
  return cotyl::FormatStr("alloc c%s: %s", c_idx, size);
}

std::string IRDeallocateCVar::ToString() const {
  return cotyl::FormatStr("dallc c%s: %s", c_idx, size);
}

template<typename T>
std::string IRLoadCVar<T>::ToString() const {
  return cotyl::FormatStr("load  v%s <-<%s> c%s", idx, detail::type_string<T>::value, c_idx);
}

template<typename T>
std::string IRStoreCVar<T>::ToString() const {
  return cotyl::FormatStr("store c%s = v%s <-<%s> v%s", c_idx, idx, detail::type_string<T>::value, src);
}

std::string IRLoadCVarAddr::ToString() const {
  return cotyl::FormatStr("addrss v%s <- c%s", idx, c_idx);
}

std::string IRReturn::ToString() const {
  if (idx) {
    return cotyl::FormatStr("retrn v%s", idx);
  }
  return "retrn void";
}

template struct IRBinop<i32>;
template struct IRBinop<u32>;
template struct IRBinop<i64>;
template struct IRBinop<u64>;
template struct IRBinop<float>;
template struct IRBinop<double>;

template struct IRUnop<i32>;
template struct IRUnop<u32>;
template struct IRUnop<i64>;
template struct IRUnop<u64>;
template struct IRUnop<float>;
template struct IRUnop<double>;

template struct IRImm<i32>;
template struct IRImm<u32>;
template struct IRImm<i64>;
template struct IRImm<u64>;
template struct IRImm<float>;
template struct IRImm<double>;

template struct IRLoadCVar<i8>;
template struct IRLoadCVar<u8>;
template struct IRLoadCVar<i16>;
template struct IRLoadCVar<u16>;
template struct IRLoadCVar<i32>;
template struct IRLoadCVar<u32>;
template struct IRLoadCVar<i64>;
template struct IRLoadCVar<u64>;
template struct IRLoadCVar<float>;
template struct IRLoadCVar<double>;

template struct IRStoreCVar<i8>;
template struct IRStoreCVar<u8>;
template struct IRStoreCVar<i16>;
template struct IRStoreCVar<u16>;
template struct IRStoreCVar<i32>;
template struct IRStoreCVar<u32>;
template struct IRStoreCVar<i64>;
template struct IRStoreCVar<u64>;
template struct IRStoreCVar<float>;
template struct IRStoreCVar<double>;

}