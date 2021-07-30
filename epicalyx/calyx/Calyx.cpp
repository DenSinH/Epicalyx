#include "Calyx.h"

#include "Format.h"


namespace epi::calyx {

std::string IRBinop::ToString() const {
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
  return cotyl::FormatStr("v%s = v%s %s v%s", idx, left_idx, op_str, right_idx);
}

template<typename T>
std::string IRImm<T>::ToString() const {
  return cotyl::FormatStr("v%s = imm<%s>", idx, value);
}

std::string IRUnop::ToString() const {
  std::string op_str;
  switch (op) {
    case Unop::Neg: op_str = "-"; break;
    case Unop::BinNot: op_str = "~"; break;
  }
  return cotyl::FormatStr("v%s = %s v%s", idx, op_str, right_idx);
}

std::string IRAllocateCVar::ToString() const {
  return cotyl::FormatStr("alloc %s: %s", c_idx, size);
}

std::string IRDeallocateCVar::ToString() const {
  return cotyl::FormatStr("dealloc %s: %s", c_idx, size);
}

std::string IRReturn::ToString() const {
  if (idx) {
    return cotyl::FormatStr("return v%s", idx);
  }
  return "return void";
}

template struct IRImm<i32>;
template struct IRImm<u32>;
template struct IRImm<i64>;
template struct IRImm<u64>;
template struct IRImm<float>;
template struct IRImm<double>;

}