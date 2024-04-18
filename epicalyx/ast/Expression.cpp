#include "Expression.h"
#include "parser/ConstParser.h"
namespace epi { struct Parser; }
#include "Initializer.h"
#include "types/Types.h"
#include "SStream.h"


namespace epi::ast {


template<typename T>
void ConstTypeVisitor::VisitValueType(const ValueType<T>& type) {
  if (type.HasValue()) {
    reduced = std::make_unique<NumericalConstantNode<T>>(type.Get());
  }
}

void ConstTypeVisitor::Visit(const ValueType<i8>& type) { VisitValueType(type); }
void ConstTypeVisitor::Visit(const ValueType<u8>& type) { VisitValueType(type); }
void ConstTypeVisitor::Visit(const ValueType<i16>& type) { VisitValueType(type); }
void ConstTypeVisitor::Visit(const ValueType<u16>& type) { VisitValueType(type); }
void ConstTypeVisitor::Visit(const ValueType<i32>& type) { VisitValueType(type); }
void ConstTypeVisitor::Visit(const ValueType<u32>& type) { VisitValueType(type); }
void ConstTypeVisitor::Visit(const ValueType<i64>& type) { VisitValueType(type); }
void ConstTypeVisitor::Visit(const ValueType<u64>& type) { VisitValueType(type); }
void ConstTypeVisitor::Visit(const ValueType<float>& type) { VisitValueType(type); }
void ConstTypeVisitor::Visit(const ValueType<double>& type) { VisitValueType(type); }

std::string FunctionCallNode::ToString() const {
  cotyl::StringStream result{};
  result << stringify(left) << '(';
  for (int i = 0; i < args.size(); i++) {
    result << stringify(args[i]);
    if (i != args.size() - 1) {
      result << ", ";
    }
  }
  result << ')';
  return result.finalize();
}

std::string MemberAccessNode::ToString() const {
  if (direct) {
    return cotyl::FormatStr("(%s).%s", left, member);
  }
  return cotyl::FormatStr("(%s)->%s", left, member);
}

pType<const CType> IdentifierNode::SemanticAnalysisImpl(const ConstParser& parser) const {
  return parser.ResolveIdentifierType(name);
}

template<typename T>
pType<const CType> NumericalConstantNode<T>::SemanticAnalysisImpl(const ConstParser&) const {
  return MakeType<ValueType<T>>(
          value, CType::LValueNess::None, CType::Qualifier::Const
  );
}

pType<const CType> StringConstantNode::SemanticAnalysisImpl(const ConstParser&) const {
  return MakeType<PointerType>(
          MakeType<ValueType<i8>>(CType::LValueNess::Assignable, CType::Qualifier::Const),
          CType::LValueNess::None,
          CType::Qualifier::Const
  );
}

pType<const CType> ArrayAccessNode::SemanticAnalysisImpl(const ConstParser& parser) const {
  return left->SemanticAnalysis(parser)->ArrayAccess(*right->SemanticAnalysis(parser));
}

pType<const CType> FunctionCallNode::SemanticAnalysisImpl(const ConstParser& parser) const {
  std::vector<pType<const CType>> call_args{};
  for (const auto& arg : args) {
    call_args.push_back(arg->SemanticAnalysis(parser));
  }
  return left->SemanticAnalysis(parser)->FunctionCall(call_args);
}

pType<const CType> MemberAccessNode::SemanticAnalysisImpl(const ConstParser& parser) const {
  if (direct) {
    return left->SemanticAnalysis(parser)->MemberAccess(member);
  }
  return left->SemanticAnalysis(parser)->Deref()->MemberAccess(member);
}

pType<const CType> TypeInitializerNode::SemanticAnalysisImpl(const ConstParser& parser) const {
  auto visitor = ValidInitializerListVisitor(parser, *list);
  type->Visit(visitor);
  return type;
}

pType<const CType> PostFixNode::SemanticAnalysisImpl(const ConstParser& parser) const {
  switch (op) {
    case TokenType::Incr: return left->SemanticAnalysis(parser)->Incr();
    case TokenType::Decr: return left->SemanticAnalysis(parser)->Decr();
    default:
      throw cotyl::FormatExceptStr("Bad AST (PostFixNode: %s)", op);
  }
}

pType<const CType> UnopNode::SemanticAnalysisImpl(const ConstParser& parser) const {
  switch (op) {
    case TokenType::Incr: return left->SemanticAnalysis(parser)->Incr();
    case TokenType::Decr: return left->SemanticAnalysis(parser)->Decr();
    case TokenType::Ampersand: return left->SemanticAnalysis(parser)->Ref();
    case TokenType::Asterisk: return left->SemanticAnalysis(parser)->Deref();
    case TokenType::Plus: return left->SemanticAnalysis(parser)->Pos();
    case TokenType::Minus: return left->SemanticAnalysis(parser)->Neg();
    case TokenType::Tilde: return left->SemanticAnalysis(parser)->BinNot();
    case TokenType::Exclamation: return left->SemanticAnalysis(parser)->LogNot();
    default:
      throw cotyl::FormatExceptStr("Bad AST (UnopNode: %s)", op);
  }
}

pType<const CType> CastNode::SemanticAnalysisImpl(const ConstParser& parser) const {
  return type->Cast(*expr->SemanticAnalysis(parser));
}

pType<const CType> BinopNode::SemanticAnalysisImpl(const ConstParser& parser) const {
  switch (op) {
    case TokenType::Asterisk: return left->SemanticAnalysis(parser)->Mul(*right->SemanticAnalysis(parser));
    case TokenType::Div: return left->SemanticAnalysis(parser)->Div(*right->SemanticAnalysis(parser));
    case TokenType::Mod: return left->SemanticAnalysis(parser)->Mod(*right->SemanticAnalysis(parser));
    case TokenType::Plus: return left->SemanticAnalysis(parser)->Add(*right->SemanticAnalysis(parser));
    case TokenType::Minus: return left->SemanticAnalysis(parser)->Sub(*right->SemanticAnalysis(parser));
    case TokenType::LShift: return left->SemanticAnalysis(parser)->LShift(*right->SemanticAnalysis(parser));
    case TokenType::RShift: return left->SemanticAnalysis(parser)->RShift(*right->SemanticAnalysis(parser));
    case TokenType::Equal: return left->SemanticAnalysis(parser)->Eq(*right->SemanticAnalysis(parser));
    case TokenType::NotEqual: return left->SemanticAnalysis(parser)->Neq(*right->SemanticAnalysis(parser));
    case TokenType::Less: return left->SemanticAnalysis(parser)->Lt(*right->SemanticAnalysis(parser));
    case TokenType::LessEqual: return left->SemanticAnalysis(parser)->Le(*right->SemanticAnalysis(parser));
    case TokenType::Greater: return left->SemanticAnalysis(parser)->Gt(*right->SemanticAnalysis(parser));
    case TokenType::GreaterEqual: return left->SemanticAnalysis(parser)->Ge(*right->SemanticAnalysis(parser));
    case TokenType::Ampersand: return left->SemanticAnalysis(parser)->BinAnd(*right->SemanticAnalysis(parser));
    case TokenType::BinOr: return left->SemanticAnalysis(parser)->BinOr(*right->SemanticAnalysis(parser));
    case TokenType::BinXor: return left->SemanticAnalysis(parser)->Xor(*right->SemanticAnalysis(parser));
    case TokenType::LogicalAnd: return left->SemanticAnalysis(parser)->LogAnd(*right->SemanticAnalysis(parser));
    case TokenType::LogicalOr: return left->SemanticAnalysis(parser)->LogOr(*right->SemanticAnalysis(parser));
    default:
      throw cotyl::FormatExceptStr("Bad AST (BinopNode: %s)", op);
  }
}

pType<const CType> TernaryNode::SemanticAnalysisImpl(const ConstParser& parser) const {
  auto cond_t = cond->SemanticAnalysis(parser);
  auto true_t = _true->SemanticAnalysis(parser);
  auto false_t = _false->SemanticAnalysis(parser);

  // todo: type conversions for constexpr
  if (cond_t->IsConstexpr()) {
    if (cond_t->ConstIntVal()) {
      return true_t;
    }
    return false_t;
  }

  return true_t->CommonType(*false_t);
}

pType<const CType> AssignmentNode::SemanticAnalysisImpl(const ConstParser& parser) const {
  auto left_t = left->SemanticAnalysis(parser);
  if (!left_t->IsAssignable()) {
    throw std::runtime_error("Cannot assign to expression");
  }
  auto right_t = right->SemanticAnalysis(parser);

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
      throw cotyl::FormatExceptStr("Bad AST (assign: %s)", op);
  }
}

pExpr MemberAccessNode::EReduce(const Parser& parser) {
  auto n_left = left->EReduce(parser);
  if (n_left) left = std::move(n_left);
  // todo: constant struct lookup
  return nullptr;
}

pExpr ArrayAccessNode::EReduce(const Parser& parser) {
  auto n_left = left->EReduce(parser);
  if (n_left) left = std::move(n_left);
  auto n_right = right->EReduce(parser);
  if (n_right) right = std::move(n_right);
  // todo: constant array lookup
  return nullptr;
}

pExpr FunctionCallNode::EReduce(const Parser& parser) {
  auto n_left = left->EReduce(parser);
  if (n_left) left = std::move(n_left);

  for (auto& arg : args) {
    auto n_arg = arg->EReduce(parser);
    if (n_arg) arg = std::move(n_arg);
  }
  // todo: consteval functions
  return nullptr;
}

pExpr TypeInitializerNode::EReduce(const Parser& parser) {
  return ReduceInitializerListVisitor(parser, *list).Reduce(*type);
}

pExpr BinopNode::EReduce(const Parser& parser) {
  auto n_left = left->EReduce(parser);
  if (n_left) left = std::move(n_left);
  auto n_right = right->EReduce(parser);
  if (n_right) right = std::move(n_right);
  return ExprNode::EReduce(parser);
}

pExpr TernaryNode::EReduce(const Parser& parser) {
  auto n_cond = cond->EReduce(parser);
  if (n_cond) cond = std::move(n_cond);
  auto n_true = _true->EReduce(parser);
  if (n_true) _true = std::move(n_true);
  auto n_false = _false->EReduce(parser);
  if (n_false) _false = std::move(n_false);

  return ExprNode::EReduce(parser);
}

pExpr AssignmentNode::EReduce(const Parser& parser) {
  auto n_left = left->EReduce(parser);
  if (n_left) left = std::move(n_left);
  auto n_right = right->EReduce(parser);
  if (n_right) right = std::move(n_right);
  // AssignmentNode may never be replaced
  return nullptr;
}

template struct NumericalConstantNode<i8>;
template struct NumericalConstantNode<u8>;
template struct NumericalConstantNode<i16>;
template struct NumericalConstantNode<u16>;
template struct NumericalConstantNode<i32>;
template struct NumericalConstantNode<u32>;
template struct NumericalConstantNode<i64>;
template struct NumericalConstantNode<u64>;
template struct NumericalConstantNode<float>;
template struct NumericalConstantNode<double>;

}