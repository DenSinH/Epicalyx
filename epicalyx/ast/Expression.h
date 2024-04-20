#pragma once

#include "Node.h"
#include "NodeVisitor.h"
namespace epi::ast { struct InitializerList; }
#include "types/BaseType.h"
#include "tokenizer/TokenType.h"
#include "Escape.h"
#include "CString.h"

#include <string>
#include <utility>
#include <vector>


namespace epi::ast {

/*
 * PRIMARY EXPRESSION
 * */

struct IdentifierNode final : ExprNode {
  ~IdentifierNode() final = default;

  IdentifierNode(cotyl::CString&& name, type::AnyType&& type) :
      ExprNode{std::move(type)}, name(std::move(name)) { }

  cotyl::CString name;

  std::string ToString() const final { return name.str(); };
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};


template<typename T>
struct NumericalConstantNode final : ExprNode {
  ~NumericalConstantNode() final = default;

  NumericalConstantNode(T value) :
      ExprNode{type::ValueType<T>{
        value, type::BaseType::LValueNess::None, type::BaseType::Qualifier::Const
      }}, value(value) { }

  const T value;

  std::string ToString() const final { return std::to_string(value); };
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};


struct StringConstantNode final : ExprNode {
  ~StringConstantNode() final = default;

  StringConstantNode(cotyl::CString&& value);

  cotyl::CString value;

  std::string ToString() const final { return cotyl::Format("\"%s\"", cotyl::Escape(value.c_str()).c_str()); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};

/*
 * POSTFIX EXPRESSION
 * */
struct ArrayAccessNode final : ExprNode {
  ~ArrayAccessNode() final = default;

  ArrayAccessNode(pExpr&& left, pExpr&& right);

  pExpr left, right;

  std::string ToString() const final { return cotyl::FormatStr("(%s)[%s]", left, right); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce() final;
};


struct FunctionCallNode final : ExprNode {
  ~FunctionCallNode() final = default;

  FunctionCallNode(pExpr&& left, std::vector<pExpr>&& args);

  pExpr left;
  std::vector<pExpr> args{};

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce() final;
};


struct MemberAccessNode final : ExprNode {
  ~MemberAccessNode() final = default;

  MemberAccessNode(pExpr&& left, bool direct, cotyl::CString&& member);

  pExpr left;
  bool direct;
  cotyl::CString member;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce() final;
};


struct TypeInitializerNode : ExprNode {

  TypeInitializerNode(type::AnyType&& type, pNode<InitializerList>&& list);

  pNode<InitializerList> list;

  std::string ToString() const final { return cotyl::FormatStr("(%s)%s", type, list); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce() final;
};


struct PostFixNode final : ExprNode {
  ~PostFixNode() final = default;

  PostFixNode(TokenType op, pExpr&& left);

  pExpr left;
  TokenType op;

  std::string ToString() const final { return cotyl::FormatStr("(%s)%s", left, op); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  // EReduce is just the constant node from the PostFixNode operation (will always be nullptr)
};


struct UnopNode final : ExprNode {
  ~UnopNode() final = default;

  UnopNode(TokenType op, pExpr&& left);

  pExpr left;
  TokenType op;

  std::string ToString() const final { return cotyl::FormatStr("%s(%s)", op, left); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  // EReduce is just the constant node from the unary operation
};


struct CastNode final : ExprNode {
  ~CastNode() final = default;

  CastNode(type::AnyType&& type, pExpr&& expr);

  pExpr expr;

  std::string ToString() const final { return cotyl::FormatStr("(%s)(%s)", type, expr); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  // EReduce is just the constant node from the cast
};


struct BinopNode final : ExprNode {
  ~BinopNode() final = default;

  BinopNode(pExpr&& left, TokenType op, pExpr&& right);

  pExpr left;
  TokenType op;
  pExpr right;

  std::string ToString() const final {
    return cotyl::FormatStr("(%s) %s (%s)", left, op, right);
  }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce() final;
};


struct TernaryNode final : ExprNode {
  ~TernaryNode() final = default;

  TernaryNode(pExpr&& cond, pExpr&& _true, pExpr&& _false);

  pExpr cond;
  pExpr _true;
  pExpr _false;

  std::string ToString() const final {
    return cotyl::FormatStr("(%s) ? (%s) : (%s)", cond, stringify(_true), _false);
  }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce() final;
};


struct AssignmentNode final : ExprNode {
  ~AssignmentNode() final = default;

  AssignmentNode(pExpr&& left, TokenType op, pExpr&& right);

  pExpr left;
  const TokenType op;
  pExpr right;

  std::string ToString() const final {
    return cotyl::FormatStr("%s %s (%s)", left, op, right);
  }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce() final;
};

}