#include "Expression.h"
#include "parser/ConstParser.h"
namespace epi { struct Parser; }
#include "Initializer.h"
#include "types/Types.h"
#include "SStream.h"


namespace epi::ast {

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

StringConstantNode::StringConstantNode(cotyl::CString&& value) :
    ExprNode{type::PointerType{
      std::make_shared<type::AnyType>(
        type::ValueType<i8>(type::BaseType::LValueNess::LValue, type::BaseType::Qualifier::Const)
      ),
      type::BaseType::LValueNess::LValue,
      type::BaseType::Qualifier::Const
    }}, 
    value(std::move(value)) { 

}

ArrayAccessNode::ArrayAccessNode(pExpr&& left, pExpr&& right) :
    ExprNode{left->type->ArrayAccess(right->type)},
    left(std::move(left)),
    right(std::move(right)) {

}

FunctionCallNode::FunctionCallNode(pExpr&& left, std::vector<pExpr>&& args) :
    ExprNode{left->type->FunctionCall(
      [&] {
        std::vector<type::AnyType> call_args{};
        call_args.reserve(args.size());
        for (const auto& arg : args) {
          call_args.push_back(arg->type);
        }
        return call_args;
      }()
    )},
    left(std::move(left)),
    args{std::move(args)} {

}

MemberAccessNode::MemberAccessNode(pExpr&& left, bool direct, cotyl::CString&& member) :
    ExprNode{direct ? left->type->MemberAccess(member) : left->type->Deref()->MemberAccess(member)},
    left(std::move(left)),
    direct(direct),
    member(std::move(member)) {

}

TypeInitializerNode::TypeInitializerNode(type::AnyType&& type, pNode<InitializerList>&& list) :
    ExprNode{std::move(type)},
    list{std::move(list)} {
  throw std::runtime_error("not reimplemented");
  // auto visitor = ValidInitializerListVisitor(*list);
  // type->Visit(visitor);
}

PostFixNode::PostFixNode(TokenType op, pExpr&& left) :
    ExprNode{left->type},
    op{op},
    left{std::move(left)} {
  // semantic analysis, postfix incr/decr type must store proper value
  op == TokenType::Incr ? left->type->Incr() : left->type->Decr();
  type->ForgetConstInfo();
}

UnopNode::UnopNode(TokenType op, pExpr&& left) :
    ExprNode{[&] {
      switch (op) {
        case TokenType::Incr: return left->type->Incr();
        case TokenType::Decr: return left->type->Decr();
        case TokenType::Ampersand: return left->type->Ref();
        case TokenType::Asterisk: return left->type->Deref();
        case TokenType::Plus: return left->type->Pos();
        case TokenType::Minus: return left->type->Neg();
        case TokenType::Tilde: return left->type->BinNot();
        case TokenType::Exclamation: return left->type->LogNot();
        default:
          throw cotyl::FormatExceptStr("Bad AST (UnopNode: %s)", op);
      }
    }()},
    op(op),
    left(std::move(left)) {

}

CastNode::CastNode(type::AnyType&& type, pExpr&& expr) :
    ExprNode{type.Cast(expr->type)},
    expr{std::move(expr)} {

}

BinopNode::BinopNode(pExpr&& left, TokenType op, pExpr&& right) :
    ExprNode{[&] {
    switch (op) {
        case TokenType::Asterisk: return left->type->Mul(right->type);
        case TokenType::Div: return left->type->Div(right->type);
        case TokenType::Mod: return left->type->Mod(right->type);
        case TokenType::Plus: return left->type->Add(right->type);
        case TokenType::Minus: return left->type->Sub(right->type);
        case TokenType::LShift: return left->type->LShift(right->type);
        case TokenType::RShift: return left->type->RShift(right->type);
        case TokenType::Equal: return left->type->Eq(right->type);
        case TokenType::NotEqual: return left->type->Neq(right->type);
        case TokenType::Less: return left->type->Lt(right->type);
        case TokenType::LessEqual: return left->type->Le(right->type);
        case TokenType::Greater: return left->type->Gt(right->type);
        case TokenType::GreaterEqual: return left->type->Ge(right->type);
        case TokenType::Ampersand: return left->type->BinAnd(right->type);
        case TokenType::BinOr: return left->type->BinOr(right->type);
        case TokenType::BinXor: return left->type->Xor(right->type);
        case TokenType::LogicalAnd: return left->type->LogAnd(right->type);
        case TokenType::LogicalOr: return left->type->LogOr(right->type);
        default:
          throw cotyl::FormatExceptStr("Bad AST (BinopNode: %s)", op);
      }
    }()},
    left(std::move(left)),
    op(op),
    right(std::move(right)) {

}

TernaryNode::TernaryNode(pExpr&& cond, pExpr&& _true, pExpr&& _false) :
    ExprNode{[&] {
      const auto& cond_t = cond->type;
      const auto& true_t = _true->type;
      const auto& false_t = _false->type;
      auto common_t = true_t.CommonType(false_t);

      // todo: type conversions for constexpr
      if (cond_t.IsConstexpr()) {
        if (cond_t.ConstIntVal()) {
          return common_t.Cast(true_t);
        }
        return common_t.Cast(false_t);
      }

      return common_t;
    }()},
    cond(std::move(cond)),
    _true(std::move(_true)),
    _false(std::move(_false)) {

}

AssignmentNode::AssignmentNode(pExpr&& left, TokenType op, pExpr&& right) :
    ExprNode{left->type.Cast([&] {
      const auto& left_t = left->type;
      if (!left_t->IsAssignable()) {
        throw std::runtime_error("Cannot assign to expression");
      }
      const auto& right_t = right->type;

      switch (op) {
        case TokenType::IMul: return left_t->Mul(right_t);
        case TokenType::IDiv: return left_t->Div(right_t);
        case TokenType::IMod: return left_t->Mod(right_t);
        case TokenType::IPlus: return left_t->Add(right_t);
        case TokenType::IMinus: return left_t->Sub(right_t);
        case TokenType::ILShift: return left_t->LShift(right_t);
        case TokenType::IRShift: return left_t->RShift(right_t);
        case TokenType::IAnd: return left_t->BinAnd(right_t);
        case TokenType::IOr: return left_t->BinOr(right_t);
        case TokenType::IXor: return left_t->Xor(right_t);
        case TokenType::Assign: return right_t;
        default:
          throw cotyl::FormatExceptStr("Bad AST (assign: %s)", op);
      }
    }())},
    left(std::move(left)),
    op(op),
    right(std::move(right)) {

}

pExpr MemberAccessNode::EReduce() {
  auto n_left = left->EReduce();
  if (n_left) left = std::move(n_left);
  // todo: constant struct lookup
  return nullptr;
}

pExpr ArrayAccessNode::EReduce() {
  auto n_left = left->EReduce();
  if (n_left) left = std::move(n_left);
  auto n_right = right->EReduce();
  if (n_right) right = std::move(n_right);
  // todo: constant array lookup
  return nullptr;
}

pExpr FunctionCallNode::EReduce() {
  auto n_left = left->EReduce();
  if (n_left) left = std::move(n_left);

  for (auto& arg : args) {
    auto n_arg = arg->EReduce();
    if (n_arg) arg = std::move(n_arg);
  }
  // todo: consteval functions
  return nullptr;
}

pExpr TypeInitializerNode::EReduce() {
  throw std::runtime_error("not reimplemented");
  // return ReduceInitializerListVisitor(parser, *list).Reduce(*type);
}

pExpr BinopNode::EReduce() {
  auto n_left = left->EReduce();
  if (n_left) left = std::move(n_left);
  auto n_right = right->EReduce();
  if (n_right) right = std::move(n_right);
  return ExprNode::EReduce();
}

pExpr TernaryNode::EReduce() {
  auto n_cond = cond->EReduce();
  if (n_cond) cond = std::move(n_cond);
  auto n_true = _true->EReduce();
  if (n_true) _true = std::move(n_true);
  auto n_false = _false->EReduce();
  if (n_false) _false = std::move(n_false);

  return ExprNode::EReduce();
}

pExpr AssignmentNode::EReduce() {
  auto n_left = left->EReduce();
  if (n_left) left = std::move(n_left);
  auto n_right = right->EReduce();
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