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
template<> const std::string type_string<Struct>::value = "struct";
template<> const std::string type_string<Pointer>::value = "pointer";

}

template<typename To, typename From>
std::string IRCast<To, From>::ToString() const {
  return cotyl::FormatStr(
      "cast  v%s <%s> <- <%s>v%s", this->idx, detail::type_string<To>::value, detail::type_string<From>::value, right_idx
  );
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
  return cotyl::FormatStr("binop v%s = v%s %s<%s> v%s", this->idx, left_idx, op_str, detail::type_string<T>::value, right_idx);
}

template<typename T>
std::string IRAddToPointer<T>::ToString() const {
  return cotyl::FormatStr("ptrad v% = v%s + %s * v%s", this->idx, ptr_idx, stride, right_idx);
}

template<typename T>
std::string IRImm<T>::ToString() const {
  return cotyl::FormatStr("immed v%s = imm<%s>(%s)", this->idx, detail::type_string<T>::value, value);
}

template<typename T>
std::string IRUnop<T>::ToString() const {
  std::string op_str;
  switch (op) {
    case Unop::Neg: op_str = "-"; break;
    case Unop::BinNot: op_str = "~"; break;
  }
  return cotyl::FormatStr("unop  v%s = %s<%s> v%s", this->idx, op_str, detail::type_string<T>::value, right_idx);
}

std::string IRAllocateCVar::ToString() const {
  return cotyl::FormatStr("alloc c%s: %s", c_idx, size);
}

std::string IRDeallocateCVar::ToString() const {
  return cotyl::FormatStr("dallc c%s: %s", c_idx, size);
}

template<typename T>
std::string IRLoadCVar<T>::ToString() const {
  return cotyl::FormatStr("load  v%s <-<%s> c%s", this->idx, detail::type_string<T>::value, c_idx);
}

template<typename T>
std::string IRStoreCVar<T>::ToString() const {
  return cotyl::FormatStr("store c%s = v%s <-<%s> v%s", c_idx, this->idx, detail::type_string<T>::value, src);
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
template struct IRBinop<Pointer>;

template struct IRAddToPointer<i32>;
template struct IRAddToPointer<u32>;
template struct IRAddToPointer<i64>;
template struct IRAddToPointer<u64>;

template struct IRUnop<i32>;
template struct IRUnop<u32>;
template struct IRUnop<i64>;
template struct IRUnop<u64>;
template struct IRUnop<float>;
template struct IRUnop<double>;
template struct IRUnop<Pointer>;  // should be unused

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
template struct IRLoadCVar<Struct>;
template struct IRLoadCVar<Pointer>;

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
template struct IRStoreCVar<Struct>;
template struct IRStoreCVar<Pointer>;

template struct IRCast<i8, i32>;
template struct IRCast<i8, u32>;
template struct IRCast<i8, i64>;
template struct IRCast<i8, u64>;
template struct IRCast<i8, float>;
template struct IRCast<i8, double>;
template struct IRCast<i8, Pointer>;
template struct IRCast<i16, i32>;
template struct IRCast<i16, u32>;
template struct IRCast<i16, i64>;
template struct IRCast<i16, u64>;
template struct IRCast<i16, float>;
template struct IRCast<i16, double>;
template struct IRCast<i16, Pointer>;
template struct IRCast<i32, i32>;
template struct IRCast<i32, u32>;
template struct IRCast<i32, i64>;
template struct IRCast<i32, u64>;
template struct IRCast<i32, float>;
template struct IRCast<i32, double>;
template struct IRCast<i32, Pointer>;
template struct IRCast<i64, i32>;
template struct IRCast<i64, u32>;
template struct IRCast<i64, i64>;
template struct IRCast<i64, u64>;
template struct IRCast<i64, float>;
template struct IRCast<i64, double>;
template struct IRCast<i64, Pointer>;
template struct IRCast<u8, i32>;
template struct IRCast<u8, u32>;
template struct IRCast<u8, i64>;
template struct IRCast<u8, u64>;
template struct IRCast<u8, float>;
template struct IRCast<u8, double>;
template struct IRCast<u8, Pointer>;
template struct IRCast<u16, i32>;
template struct IRCast<u16, u32>;
template struct IRCast<u16, i64>;
template struct IRCast<u16, u64>;
template struct IRCast<u16, float>;
template struct IRCast<u16, double>;
template struct IRCast<u16, Pointer>;
template struct IRCast<u32, i32>;
template struct IRCast<u32, u32>;
template struct IRCast<u32, i64>;
template struct IRCast<u32, u64>;
template struct IRCast<u32, float>;
template struct IRCast<u32, double>;
template struct IRCast<u32, Pointer>;
template struct IRCast<u64, i32>;
template struct IRCast<u64, u32>;
template struct IRCast<u64, i64>;
template struct IRCast<u64, u64>;
template struct IRCast<u64, float>;
template struct IRCast<u64, double>;
template struct IRCast<u64, Pointer>;
template struct IRCast<float, i32>;
template struct IRCast<float, u32>;
template struct IRCast<float, i64>;
template struct IRCast<float, u64>;
template struct IRCast<float, float>;
template struct IRCast<float, double>;
template struct IRCast<float, Pointer>;
template struct IRCast<double, i32>;
template struct IRCast<double, u32>;
template struct IRCast<double, i64>;
template struct IRCast<double, u64>;
template struct IRCast<double, float>;
template struct IRCast<double, double>;
template struct IRCast<double, Pointer>;
template struct IRCast<Pointer, i32>;
template struct IRCast<Pointer, u32>;
template struct IRCast<Pointer, i64>;
template struct IRCast<Pointer, u64>;
template struct IRCast<Pointer, float>;
template struct IRCast<Pointer, double>;
template struct IRCast<Pointer, Pointer>;

}