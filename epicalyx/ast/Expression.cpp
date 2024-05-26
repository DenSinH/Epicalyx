#include "Expression.h"
#include "Vector.h"                    // for vec_iterator
#include <memory>                      // for unique_ptr, make_shared, swap
#include <utility>                     // for move
#include "Default.h"                   // for i8, i16, i32, i64, u16, u32, u64
#include "Escape.h"                    // for QuotedEscape
#include "Format.h"                    // for FormatStr, FormatExceptStr, Join
#include "Initializer.h"               // for InitializerList
#include "SStream.h"                   // for StringStream
#include "Stringify.h"                 // for stringify
#include "Variant.h"                   // for Variant
#include "ast/Node.h"                  // for ExprNode, ASTError, stringify
#include "tokenizer/TokenType.h"       // for TokenType
#include "types/AnyType.h"             // for AnyType
#include "types/BaseType.h"            // for BaseType, LValue, Qualifier
#include "types/Types.h"               // for ValueType, ArrayType, PointerT...


namespace epi::ast {

  
IdentifierNode::IdentifierNode(cotyl::CString&& name, type::AnyType&& type) :
    ExprNode{std::move(type)}, name(std::move(name)) { 

}

std::string IdentifierNode::ToString() const { 
  return name.str(); 
};


template<typename T>
NumericalConstantNode<T>::NumericalConstantNode(T value) :
    ExprNode{type::ValueType<T>{
      value, type::LValue::None, type::Qualifier::Const
    }}, value(value) { 

}

template<typename T>
std::string NumericalConstantNode<T>::ToString() const { 
  return std::to_string(value); 
};


StringConstantNode::StringConstantNode(cotyl::CString&& value) :
    ExprNode{type::ArrayType{
      std::make_shared<type::AnyType>(
        type::ValueType<i8>(type::LValue::LValue, type::Qualifier::Const)
      ),
      value.size() + 1  // include \0 terminator
    }}, 
    value(std::move(value)) { 

}

std::string StringConstantNode::ToString() const { 
  return cotyl::QuotedEscape(value.c_str()).finalize();
}


ArrayAccessNode::ArrayAccessNode(pExpr&& left, pExpr&& right) :
    ExprNode{left->type->ArrayAccess(right->type)},
    ptr{std::move(left)},
    offs{std::move(right)} {
  if (!(ptr->type.holds_alternative<type::PointerType>() || ptr->type.holds_alternative<type::ArrayType>())) {
    std::swap(ptr, offs);
  }
}

std::string ArrayAccessNode::ToString() const { 
  return cotyl::FormatStr("(%s)[%s]", ptr, offs); 
}


FunctionCallNode::FunctionCallNode(pExpr&& left, cotyl::vector<pExpr>&& args) :
    ExprNode{left->type->FunctionCall(
      [&] {
        cotyl::vector<type::AnyType> call_args{};
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


MemberAccessNode::MemberAccessNode(pExpr&& left, cotyl::CString&& member) :
    ExprNode{left->type->MemberAccess(member)},
    left(std::move(left)),
    member(std::move(member)) {

}

std::string MemberAccessNode::ToString() const {
  return cotyl::FormatStr("(%s).%s", left, member);
}


TypeInitializerNode::TypeInitializerNode(type::AnyType&& type, InitializerList&& list) :
    ExprNode{std::move(type)},
    list{std::move(list)} {
  this->list.ValidateAndReduce(type);
}

std::string TypeInitializerNode::ToString() const { 
  return cotyl::FormatStr("(%s)%s", type, list); 
}

PostFixNode::PostFixNode(TokenType op, pExpr&& left) :
    ExprNode{left->type},
    op{op},
    left{std::move(left)} {
  // semantic analysis, postfix incr/decr type must store proper value
  op == TokenType::Incr ? type->Incr() : type->Decr();

  // don't propagate constant info in "assigning" expression
  type->ForgetConstInfo();
}

std::string PostFixNode::ToString() const { 
  return cotyl::FormatStr("(%s)%s", left, op); 
}


UnopNode::UnopNode(TokenType op, pExpr&& left) :
    ExprNode{[&]() -> type::AnyType {
      switch (op) {
        case TokenType::Incr: return left->type->Incr();
        case TokenType::Decr: return left->type->Decr();
        case TokenType::Ampersand: return left->type.Ref();
        case TokenType::Asterisk: return left->type->Deref();
        case TokenType::Plus: return left->type->Pos();
        case TokenType::Minus: return left->type->Neg();
        case TokenType::Tilde: return left->type->BinNot();
        case TokenType::Exclamation: return left->type->LogNot();
        default:
          throw cotyl::FormatExceptStr<ASTError>("UnopNode: %s", op);
      }
    }()},
    op(op),
    left(std::move(left)) {
  switch (op) {
    case TokenType::Incr: case TokenType::Decr:
    case TokenType::Ampersand: case TokenType::Asterisk:
      type->ForgetConstInfo();
    default: break;
  }
}

std::string UnopNode::ToString() const { 
  return cotyl::FormatStr("%s(%s)", op, left); 
}


CastNode::CastNode(type::AnyType&& type, pExpr&& expr) :
    ExprNode{type.Cast(expr->type)},
    expr{std::move(expr)} {

}

std::string CastNode::ToString() const { 
  return cotyl::FormatStr("(%s)(%s)", type, expr); 
}


BinopNode::BinopNode(pExpr&& left, TokenType op, pExpr&& right) :
    ExprNode{[&]() -> type::AnyType {
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
          throw cotyl::FormatExceptStr<ASTError>("BinopNode: %s", op);
      }
    }()},
    left(std::move(left)),
    op(op),
    right(std::move(right)) {

}

std::string BinopNode::ToString() const {
  return cotyl::FormatStr("(%s) %s (%s)", left, op, right);
}


TernaryNode::TernaryNode(pExpr&& cond, pExpr&& _true, pExpr&& _false) :
    ExprNode{[&] {
      const auto& cond_t = cond->type;
      const auto& true_t = _true->type;
      const auto& false_t = _false->type;
      auto common_t = true_t.CommonType(false_t);

      if (cond->IsConstexpr()) {
        if (cond->ConstBoolVal()) {
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

std::string TernaryNode::ToString() const {
  return cotyl::FormatStr("(%s) ? (%s) : (%s)", cond, stringify(_true), _false);
}


AssignmentNode::AssignmentNode(pExpr&& left, TokenType op, pExpr&& right) :
    ExprNode{left->type.Cast([&] {
      const auto& left_t = left->type;
      if (!left_t->IsAssignable()) {
        throw type::TypeError("Cannot assign to expression");
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
          throw cotyl::FormatExceptStr<ASTError>("Assignment: %s", op);
      }
    }())},
    left(std::move(left)),
    op(op),
    right(std::move(right)) {
  // don't propagate constant info in assignment
  type->ForgetConstInfo();
}

std::string AssignmentNode::ToString() const {
  return cotyl::FormatStr("%s %s (%s)", left, op, right);
}


ExpressionListNode::ExpressionListNode(cotyl::vector<pExpr>&& exprs) :
    ExprNode{exprs.back()->type},
    exprs{std::move(exprs)} {
  // todo: this is too strict, should only forget constinfo
  // if no statement has any effect
  type->ForgetConstInfo();
}

std::string ExpressionListNode::ToString() const {
  return cotyl::Join(", ", exprs);
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