#pragma once

#include "Node.h"
#include "NodeVisitor.h"
#include "Initializer.h"
#include "tokenizer/TokenType.h"
#include "CString.h"
#include "Vector.h"

#include <string>
#include <utility>


namespace epi::ast {

/*
 * PRIMARY EXPRESSION
 * */

struct IdentifierNode final : ExprNode {
  ~IdentifierNode() final = default;

  IdentifierNode(cotyl::CString&& name, type::AnyType&& type);

  cotyl::CString name;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};


template<typename T>
struct NumericalConstantNode final : ExprNode {
  ~NumericalConstantNode() final = default;

  NumericalConstantNode(T value);

  const T value;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};


struct StringConstantNode final : ExprNode {
  ~StringConstantNode() final = default;

  StringConstantNode(cotyl::CString&& value);

  cotyl::CString value;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};

/*
 * POSTFIX EXPRESSION
 * */
struct ArrayAccessNode final : ExprNode {
  ~ArrayAccessNode() final = default;

  ArrayAccessNode(pExpr&& left, pExpr&& right);

  // in the constructor, we guarantee that "ptr" will
  // hold the pointer type
  pExpr ptr, offs;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};


struct FunctionCallNode final : ExprNode {
  ~FunctionCallNode() final = default;

  FunctionCallNode(pExpr&& left, cotyl::vector<pExpr>&& args);

  pExpr left;
  cotyl::vector<pExpr> args{};

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};


struct MemberAccessNode final : ExprNode {
  ~MemberAccessNode() final = default;

  MemberAccessNode(pExpr&& left, bool direct, cotyl::CString&& member);

  pExpr left;
  bool direct;
  cotyl::CString member;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};


struct TypeInitializerNode : ExprNode {

  TypeInitializerNode(type::AnyType&& type, InitializerList&& list);

  InitializerList list;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};


struct PostFixNode final : ExprNode {
  ~PostFixNode() final = default;

  PostFixNode(TokenType op, pExpr&& left);

  pExpr left;
  TokenType op;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};


struct UnopNode final : ExprNode {
  ~UnopNode() final = default;

  UnopNode(TokenType op, pExpr&& left);

  pExpr left;
  TokenType op;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};


struct CastNode final : ExprNode {
  ~CastNode() final = default;

  CastNode(type::AnyType&& type, pExpr&& expr);

  pExpr expr;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};


struct BinopNode final : ExprNode {
  ~BinopNode() final = default;

  BinopNode(pExpr&& left, TokenType op, pExpr&& right);

  pExpr left;
  TokenType op;
  pExpr right;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};


struct TernaryNode final : ExprNode {
  ~TernaryNode() final = default;

  TernaryNode(pExpr&& cond, pExpr&& _true, pExpr&& _false);

  pExpr cond;
  pExpr _true;
  pExpr _false;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};


struct AssignmentNode final : ExprNode {
  ~AssignmentNode() final = default;

  AssignmentNode(pExpr&& left, TokenType op, pExpr&& right);

  pExpr left;
  const TokenType op;
  pExpr right;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};

}