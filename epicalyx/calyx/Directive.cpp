#include "Directive.h"
#include "Format.h"
#include "Decltype.h"


namespace epi::cotyl {

template<> calyx::detail::any_expr_t::~Variant() = default;
template<> calyx::detail::any_directive_t::~Variant() = default;

}

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
template<> const std::string type_string<Pointer>::value = "pointer";
template<> const std::string type_string<Aggregate>::value = "aggregate";

std::string TypeString(const Local::Type& type) {
  switch (type) {
    case Local::Type::I8: return type_string<i8>::value;
    case Local::Type::U8: return type_string<u8>::value;
    case Local::Type::I16: return type_string<i16>::value;
    case Local::Type::U16: return type_string<u16>::value;
    case Local::Type::I32: return type_string<i32>::value;
    case Local::Type::U32: return type_string<u32>::value;
    case Local::Type::I64: return type_string<i64>::value;
    case Local::Type::U64: return type_string<u64>::value;
    case Local::Type::Float: return type_string<float>::value;
    case Local::Type::Double: return type_string<double>::value;
    case Local::Type::Pointer: return type_string<Pointer>::value;
    case Local::Type::Aggregate: return type_string<Aggregate>::value;
  }
}

}

std::string NoOp::ToString() const {
  return "noop ";
}

template<typename To, typename From>
requires (
  is_calyx_type_v<From> && 
  (is_calyx_type_v<To> || is_calyx_small_type_v<To>)
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
  if (right.IsVar()) {
    return cotyl::FormatStr(
      "binop v%s = v%s %s<%s> v%s",
      this->idx, left_idx, op_str, detail::type_string<T>::value, right.GetVar()
    );
  }
  else {
    return cotyl::FormatStr(
      "binim v%s = v%s %s<%s> %s", 
      this->idx, left_idx, op_str, detail::type_string<T>::value, right.GetScalar()
    );
  }
}

template<typename T>
requires (is_calyx_integral_type_v<T>)
std::string Shift<T>::ToString() const {
  std::string op_str;
  switch (op) {
    case ShiftType::Left: op_str = "<<"; break;
    case ShiftType::Right: op_str = ">>"; break;
  }
  if (right.IsVar()) {
    if (left.IsVar()) {
      return cotyl::FormatStr(
        "shift v%s = v%s %s<%s> v%s",
        this->idx, left.GetVar(), op_str, detail::type_string<T>::value, right.GetVar()
      );
    }
    else {
      return cotyl::FormatStr(
        "shift v%s = imm(%s) %s<%s> v%s",
        this->idx, left.GetScalar(), op_str, detail::type_string<T>::value, right.GetVar()
      );
    }
  }
  else {
    if (left.IsVar()) {
      return cotyl::FormatStr(
        "shift v%s = v%s %s<%s> imm(%s)",
        this->idx, left.GetVar(), op_str, detail::type_string<T>::value, right.GetScalar()
      );
    }
    else {
      return cotyl::FormatStr(
        "shift v%s = imm(%s) %s<%s> imm(%s)",
        this->idx, left.GetScalar(), op_str, detail::type_string<T>::value, right.GetScalar()
      );
    }
  }
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

template<typename T>
requires (is_calyx_type_v<T>)
std::string Compare<T>::ToString() const {
  if (right.IsVar()) {
    return cotyl::FormatStr(
        "cmp   v%s = v%s %s<%s> v%s",
        this->idx, left_idx, cmp_string(op),
        detail::type_string<T>::value, right.GetVar()
    );
  }
  else {
    return cotyl::FormatStr(
        "cmpim v%s = v%s %s<%s> %s",
        this->idx, left_idx, cmp_string(op),
        detail::type_string<T>::value, right.GetScalar()
    );
  }
}

std::string UnconditionalBranch::ToString() const {
  return cotyl::FormatStr("brnch L%s", this->dest);
}

template<typename T>
requires (is_calyx_type_v<T>)
std::string BranchCompare<T>::ToString() const {
  if (right.IsVar()) {
    return cotyl::FormatStr(
        "brnch (v%s %s<%s> v%s) ? L%s : L%s",
        left_idx, cmp_string(op), detail::type_string<T>::value, right.GetVar(),
        tdest, fdest
    );
  }
  else {
    return cotyl::FormatStr(
        "brnch (v%s %s<%s> imm(%s)) ? L%s : L%s",
        left_idx, cmp_string(op), detail::type_string<T>::value, right.GetScalar(),
        tdest, fdest
    );
  }
}

std::string Select::ToString() const {
  return cotyl::FormatStr("selct v%s", idx);
}

template<typename T>
requires (is_calyx_integral_type_v<T>)
std::string AddToPointer<T>::ToString() const {
  if (ptr.IsVar()) {
    if (right.IsVar()) {
      return cotyl::FormatStr(
        "ptrad v%s = v%s + %s * v%s", 
        this->idx, ptr.GetVar(), stride, right.GetVar()
      );
    }
    else {
      return cotyl::FormatStr(
        "paddi v%s = v%s + %s * imm(%s)", 
        this->idx, ptr.GetVar(), stride, right.GetScalar()
      );
    }
  }
  else {
    if (right.IsVar()) {
      return cotyl::FormatStr(
        "ptrad v%s = %s + %s * v%s", 
        this->idx, ptr.GetScalar(), stride, right.GetVar()
      );
    }
    else {
      return cotyl::FormatStr(
        "paddi v%s = %s + %s * imm(%s)", 
        this->idx, ptr.GetScalar(), stride, right.GetScalar()
      );
    }
  }
  
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

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
std::string LoadLocal<T>::ToString() const {
  return cotyl::FormatStr("load  v%s <-<%s> c%s", this->idx, detail::type_string<T>::value, loc_idx);
}

std::string LoadLocalAddr::ToString() const {
  return cotyl::FormatStr("addrs v%s <- c%s", idx, loc_idx);
}

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
std::string StoreLocal<T>::ToString() const {
  if (this->src.IsVar()) {
    return cotyl::FormatStr(
      "store c%s = <%s> v%s", 
      loc_idx, detail::type_string<T>::value, this->src.GetVar()
    );
  }
  else {
    return cotyl::FormatStr(
      "strim c%s = <%s> imm(%s)", 
      loc_idx, detail::type_string<T>::value, this->src.GetScalar()
    );
  }
}

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
std::string LoadGlobal<T>::ToString() const {
  return cotyl::FormatStr("lglob v%s <-<%s> [%s]", this->idx, detail::type_string<T>::value, symbol);
}

std::string LoadGlobalAddr::ToString() const {
  return cotyl::FormatStr("adglb v%s <- &[%s]", idx, symbol);
}

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
std::string StoreGlobal<T>::ToString() const {
  if (this->src.IsVar()) {
    return cotyl::FormatStr(
      "sglob [%s] = <%s> v%s",
      symbol, detail::type_string<T>::value, this->src.GetVar()
    );
  }
  else {
    return cotyl::FormatStr(
      "sglim [%s] = <%s> %s",
      symbol, detail::type_string<T>::value, this->src.GetScalar()
    );
  }
}

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
std::string LoadFromPointer<T>::ToString() const {
  return cotyl::FormatStr("deref v%s <-<%s> *v%s", this->idx, detail::type_string<T>::value, ptr_idx);
}

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_memory_types>)
std::string StoreToPointer<T>::ToString() const {
  if (this->src.IsVar()) {
    return cotyl::FormatStr(
      "store *v%s <-<%s> v%s",
      ptr_idx, detail::type_string<T>::value, this->src.GetVar()
    );
  }
  else {
    return cotyl::FormatStr(
      "strim *v%s <-<%s> imm(%s)",
      ptr_idx, detail::type_string<T>::value, this->src.GetScalar()
    );
  }
}

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_return_types>)
std::string Return<T>::ToString() const {
  if constexpr(std::is_same_v<T, void>) {
    return "retrn void";
  }
  else {
    if (val.IsVar()) {
      return cotyl::FormatStr(
        "retrn [%s]v%s", 
        detail::type_string<T>::value, val.GetVar()
      );
    }
    else {
      return cotyl::FormatStr(
        "retrn [%s]imm(%s)", 
        detail::type_string<T>::value, val.GetScalar()
      );
    }
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
requires (cotyl::pack_contains_v<T, calyx_return_types>)
std::string Call<T>::ToString() const {

  if constexpr(std::is_same_v<T, void>) {
    if (!args->var_args.empty()) {
      return cotyl::FormatStr("call  [void]v%s(%s, ... %s)", fn_idx, make_args_list(args->args), make_args_list(args->var_args));
    }
    else {
      return cotyl::FormatStr("call  [void]v%s(%s)", fn_idx, make_args_list(args->args));
    }
  }
  else {
    if (!args->var_args.empty()) {
      return cotyl::FormatStr("call  v%s <- [%s]v%s(%s, ... %s)",
                              idx, detail::type_string<T>::value, fn_idx, make_args_list(args->args), make_args_list(args->var_args)
      );
    }
    else {
      return cotyl::FormatStr("call  v%s <- [%s]v%s(%s)", idx, detail::type_string<T>::value, fn_idx, make_args_list(args->args));
    }
  }
}

template<typename T>
requires (cotyl::pack_contains_v<T, calyx_return_types>)
std::string CallLabel<T>::ToString() const {

  if constexpr(std::is_same_v<T, void>) {
    if (!args->var_args.empty()) {
      return cotyl::FormatStr("call  [void]%s(%s, ... %s)", label, make_args_list(args->args), make_args_list(args->var_args));
    }
    else {
      return cotyl::FormatStr("call  [void]%s(%s)", label, make_args_list(args->args));
    }
  }
  else {
    if (!args->var_args.empty()) {
      return cotyl::FormatStr("call  v%s <- [%s]%s(%s, ... %s)",
                              idx, detail::type_string<T>::value, label, make_args_list(args->args), make_args_list(args->var_args)
      );
    }
    else {
      return cotyl::FormatStr("call  v%s <- [%s]%s(%s)", idx, detail::type_string<T>::value, label, make_args_list(args->args));
    }
  }
}

STRINGIFY_METHOD(AnyExpr) {
  return value.visit<std::string>([](const auto& dir) -> std::string { 
    return dir.ToString(); 
  });
}

STRINGIFY_METHOD(AnyDirective) {
  return value.visit<std::string>([](const auto& dir) -> std::string { 
    return dir.ToString(); 
  });
}

// force instantiation of all directives
template<typename...>
struct DirectiveInstantiator;

template<typename... Ts>
struct DirectiveInstantiator<cotyl::pack<Ts...>> : public Ts... {};

template struct DirectiveInstantiator<detail::directive_pack>;

bool IsBlockEnd(const AnyDirective& dir) {
  return dir.visit<bool>(
    [&](const auto& dir) {
      using dir_t = decltype_t(dir);
      if constexpr(std::is_base_of_v<calyx::Branch, dir_t>) {
        return true;
      }
      else if constexpr(cotyl::is_instantiation_of_v<calyx::Return, dir_t>) {
        return true;
      }
      return false;
    }
  );
}
// static_assert(sizeof(AnyDirective) == 12);

}