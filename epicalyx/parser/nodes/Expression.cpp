#include "Expression.h"
#include "parser/Parser.h"
#include "types/Types.h"

#include <sstream>


namespace epi {

std::string FunctionCall::to_string() const {
  std::stringstream result{};
  result << left->to_string() << '(';
  for (int i = 0; i < args.size(); i++) {
    result << args[i]->to_string();
    if (i != args.size() - 1) {
      result << ", ";
    }
  }
  result << ')';
  return result.str();
}


pType<const CType> Identifier::GetType(Parser& parser) const {
  if (parser.enum_values.Has(name)) {
    return MakeType<ValueType<Parser::enum_type>>(
            parser.enum_values.Get(name),
            CType::LValueNess::None,
            CType::Qualifier::Const
    );
  }
  else if (parser.variables.Has(name)) {
    return parser.variables.Get(name);
  }
  else {
    throw cotyl::FormatExceptStr("Undeclared identifier: '%s'", name);
  }
}

template<typename T>
pType<const CType> NumericalConstant<T>::GetType(Parser&) const {
  return MakeType<ValueType<T>>(
          value, CType::LValueNess::None, CType::Qualifier::Const
  );
}

pType<const CType> StringConstant::GetType(Parser&) const {
  return MakeType<PointerType>(
          MakeType<ValueType<i8>>(CType::LValueNess::Assignable, CType::Qualifier::Const),
          CType::LValueNess::None,
          CType::Qualifier::Const
  );
}

pType<const CType> ArrayAccess::GetType(Parser& parser) const {
  return left->GetType(parser)->ArrayAccess(*right->GetType(parser));
}

pType<const CType> FunctionCall::GetType(Parser& parser) const {
  std::vector<pType<const CType>> call_args{};
  for (const auto& arg : args) {
    call_args.push_back(arg->GetType(parser));
  }
  return left->GetType(parser)->FunctionCall(call_args);
}

pType<const CType> MemberAccess::GetType(Parser& parser) const {
  return left->GetType(parser)->MemberAccess(member);
}

pType<const CType> TypeInitializer::GetType(Parser&) const {
  // todo: check initializer list correctness
  return type;
}

pType<const CType> PostFix::GetType(Parser& parser) const {
  switch (op) {
    case TokenType::Incr: return left->GetType(parser)->Incr();
    case TokenType::Decr: return left->GetType(parser)->Decr();
    default:
      throw cotyl::FormatExceptStr("Bad AST (postfix: %s)", Token(op));
  }
}

pType<const CType> Unary::GetType(Parser& parser) const {
  switch (op) {
    case TokenType::Incr: return left->GetType(parser)->Incr();
    case TokenType::Decr: return left->GetType(parser)->Decr();
    case TokenType::Ampersand: return left->GetType(parser)->Ref();
    case TokenType::Asterisk: return left->GetType(parser)->Deref();
    case TokenType::Plus: return left->GetType(parser)->Pos();
    case TokenType::Minus: return left->GetType(parser)->Neg();
    case TokenType::Tilde: return left->GetType(parser)->BinNot();
    case TokenType::Exclamation: return left->GetType(parser)->LogNot();
    default:
      throw cotyl::FormatExceptStr("Bad AST (unary: %s)", Token(op));
  }
}

pType<const CType> Cast::GetType(Parser& parser) const {
  if (!type->Cast(*expr->GetType(parser))) {
    throw std::runtime_error("Cannot cast expression to type");
  }
  return type;
}

pType<const CType> Binop::GetType(Parser& parser) const {
  switch (op) {
    case TokenType::Asterisk: return left->GetType(parser)->Mul(*right->GetType(parser));
    case TokenType::Div: return left->GetType(parser)->Div(*right->GetType(parser));
    case TokenType::Mod: return left->GetType(parser)->Mod(*right->GetType(parser));
    case TokenType::Plus: return left->GetType(parser)->Add(*right->GetType(parser));
    case TokenType::Minus: return left->GetType(parser)->Sub(*right->GetType(parser));
    case TokenType::LShift: return left->GetType(parser)->LShift(*right->GetType(parser));
    case TokenType::RShift: return left->GetType(parser)->RShift(*right->GetType(parser));
    case TokenType::Equal: return left->GetType(parser)->Eq(*right->GetType(parser));
    case TokenType::NotEqual: return left->GetType(parser)->Neq(*right->GetType(parser));
    case TokenType::Ampersand: return left->GetType(parser)->BinAnd(*right->GetType(parser));
    case TokenType::BinOr: return left->GetType(parser)->BinOr(*right->GetType(parser));
    case TokenType::BinXor: return left->GetType(parser)->Xor(*right->GetType(parser));
    case TokenType::LogicalAnd: return left->GetType(parser)->LogAnd(*right->GetType(parser));
    case TokenType::LogicalOr: return left->GetType(parser)->LogOr(*right->GetType(parser));
    default:
      throw cotyl::FormatExceptStr("Bad AST (binop: %s)", Token(op));
  }
}

pType<const CType> Ternary::GetType(Parser& parser) const {
  auto cond_t = cond->GetType(parser);
  auto true_t = _true->GetType(parser);
  auto false_t = _false->GetType(parser);

  // todo: type conversions
  if (cond_t->IsConstexpr()) {
    if (cond_t->ConstIntVal()) {
      return true_t;
    }
    return false_t;
  }
  return true_t->CommonType(*false_t);
}

pType<const CType> Assignment::GetType(Parser& parser) const {
  auto left_t = left->GetType(parser);
  if (!left_t->IsAssignable()) {
    throw std::runtime_error("Cannot assign to expression");
  }
  auto right_t = right->GetType(parser);

  switch (op) {
    case TokenType::IMul: return left_t->Mul(*right_t);
    case TokenType::IDiv: return left_t->Div(*right_t);
    case TokenType::IMod: return left_t->Mod(*right_t);
    case TokenType::IPlus: return left_t->Add(*right_t);
    case TokenType::IMinus: return left_t->Sub(*right_t);
    case TokenType::ILShift: return left_t->LShift(*right_t);
    case TokenType::IRShift: return left_t->RShift(*right_t);
    case TokenType::IAnd: return left_t->BinAnd(*right_t);
    case TokenType::IOr: return left_t->BinOr(*right_t);
    case TokenType::IXor: return left_t->Xor(*right_t);
    case TokenType::Assign: {
      if (!left_t->Cast(*right_t)) {
        throw std::runtime_error("Cannot assign value to type");
      }
      return left_t;
    }
    default:
      throw cotyl::FormatExceptStr("Bad AST (assign: %s)", Token(op));
  }
}

template struct NumericalConstant<i32>;
template struct NumericalConstant<i64>;
template struct NumericalConstant<u32>;
template struct NumericalConstant<u64>;
template struct NumericalConstant<float>;
template struct NumericalConstant<double>;

}