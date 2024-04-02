#pragma once

#include "Node.h"
#include "NodeVisitor.h"
#include "Initializer.h"
#include "types/EpiCType.h"
#include "tokenizer/Token.h"
#include "Escape.h"

#include <string>
#include <utility>
#include <vector>
#include <variant>


namespace epi::ast {

struct ConstTypeVisitor : public TypeVisitor {
  ConstTypeVisitor(const Parser& parser) : parser(parser) { }

  const Parser& parser;
  pExpr reduced = nullptr;

  pExpr GetConstNode(const CType& type) {
    type.Visit(*this);
    return std::move(reduced);
  }

  template<typename T>
  void VisitValueType(const ValueType<T>& type);

  void Visit(const VoidType& type) final { }
  void Visit(const ValueType<i8>& type) final;
  void Visit(const ValueType<u8>& type) final;
  void Visit(const ValueType<i16>& type) final;
  void Visit(const ValueType<u16>& type) final;
  void Visit(const ValueType<i32>& type) final;
  void Visit(const ValueType<u32>& type) final;
  void Visit(const ValueType<i64>& type) final;
  void Visit(const ValueType<u64>& type) final;
  void Visit(const ValueType<float>& type) final;
  void Visit(const ValueType<double>& type) final;
  void Visit(const PointerType& type) final { }
  void Visit(const ArrayType& type) final { }
  void Visit(const FunctionType& type) final { }
  void Visit(const StructType& type) final { }
  void Visit(const UnionType& type) final { }
};

/*
 * PRIMARY EXPRESSION
 * */

struct IdentifierNode final : public ExprNode {
  ~IdentifierNode() final = default;

  IdentifierNode(std::string name) :
      name(std::move(name)) {

  }

  const std::string name;

  std::string ToString() const final { return name; };
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce(const Parser& parser) final { return nullptr; }

protected:
  pType<const CType> SemanticAnalysisImpl(const ConstParser& parser) const final;
};


template<typename T>
struct NumericalConstantNode final : public ExprNode {
  ~NumericalConstantNode() final = default;

  NumericalConstantNode(T value) :
      value(value) {

  }

  const T value;

  std::string ToString() const final { return std::to_string(value); };
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce(const Parser& parser) final { return nullptr; }

protected:
  pType<const CType> SemanticAnalysisImpl(const ConstParser& parser) const final;
};


struct StringConstantNode final : public ExprNode {
  ~StringConstantNode() final = default;

  StringConstantNode(std::string value) :
      value(std::move(value)) {

  }

  const std::string value;

  std::string ToString() const final { return cotyl::FormatStr("\"%s\"", cotyl::Escape(value)); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce(const Parser& parser) final { return nullptr; }

protected:
  pType<const CType> SemanticAnalysisImpl(const ConstParser& parser) const final;
};

/*
 * POSTFIX EXPRESSION
 * */
struct ArrayAccessNode final : public ExprNode {
  ~ArrayAccessNode() final = default;

  ArrayAccessNode(pExpr&& left, pExpr&& right) :
      left(std::move(left)),
      right(std::move(right)) {

  }

  pExpr left, right;

  std::string ToString() const final { return cotyl::FormatStr("(%s)[%s]", left, right); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce(const Parser& parser) final;

protected:
  pType<const CType> SemanticAnalysisImpl(const ConstParser& parser) const final;
};


struct FunctionCallNode final : public ExprNode {
  ~FunctionCallNode() final = default;

  FunctionCallNode(pExpr&& left) :
      left(std::move(left)) {

  }

  void AddArg(pExpr&& arg) {
    args.emplace_back(std::move(arg));
  }

  pExpr left;
  std::vector<pExpr> args{};

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce(const Parser& parser) final;

protected:
  pType<const CType> SemanticAnalysisImpl(const ConstParser& parser) const final;
};


struct MemberAccessNode final : public ExprNode {
  ~MemberAccessNode() final = default;

  MemberAccessNode(pExpr&& left, bool direct, std::string member) :
      left(std::move(left)),
      direct(direct),
      member(std::move(member)) {

  }

  pExpr left;
  bool direct;
  const std::string member;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce(const Parser& parser) final;

protected:
  pType<const CType> SemanticAnalysisImpl(const ConstParser& parser) const final;
};


struct TypeInitializerNode : public ExprNode {

  TypeInitializerNode(pType<const CType> type, pNode<InitializerList>&& list) :
      type(std::move(type)),
      list(std::move(list)) {

  }

  pType<const CType> type;
  pNode<InitializerList> list;

  std::string ToString() const final { return cotyl::FormatStr("(%s)%s", type, list); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce(const Parser& parser) final;

protected:
  pType<const CType> SemanticAnalysisImpl(const ConstParser& parser) const final;
};


struct PostFixNode final : public ExprNode {
  ~PostFixNode() final = default;

  PostFixNode(const TokenType op, pExpr&& left) :
      op(op),
      left(std::move(left)) {

  }

  const pExpr left;
  const TokenType op;

  std::string ToString() const final { return cotyl::FormatStr("(%s)%s", left, Token(op)); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  // EReduce is just the constant node from the PostFixNode operation (will always be nullptr)

protected:
  pType<const CType> SemanticAnalysisImpl(const ConstParser& parser) const final;
};


struct UnopNode final : public ExprNode {
  ~UnopNode() final = default;

  UnopNode(TokenType op, pExpr&& left) :
      op(op),
      left(std::move(left)) {

  }

  const pExpr left;
  const TokenType op;

  std::string ToString() const final { return cotyl::FormatStr("%s(%s)", Token(op), left); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  // EReduce is just the constant node from the unary operation

protected:
  pType<const CType> SemanticAnalysisImpl(const ConstParser& parser) const final;
};


struct CastNode final : public ExprNode {
  ~CastNode() final = default;

  CastNode(pType<const CType> type, pExpr&& expr) :
      type(std::move(type)),
      expr(std::move(expr)) {

  }

  const pType<const CType> type;
  const pExpr expr;

  std::string ToString() const final { return cotyl::FormatStr("(%s)(%s)", type, expr); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  // EReduce is just the constant node from the cast

protected:
  pType<const CType> SemanticAnalysisImpl(const ConstParser& parser) const final;
};


struct BinopNode final : public ExprNode {
  ~BinopNode() final = default;

  BinopNode(pExpr&& left, const TokenType op, pExpr&& right) :
      left(std::move(left)),
      op(op),
      right(std::move(right)) {

  }

  pExpr left;
  const TokenType op;
  pExpr right;

  std::string ToString() const final {
    return cotyl::FormatStr("(%s) %s (%s)", left, Token(op), right);
  }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce(const Parser& parser) final;

protected:
  pType<const CType> SemanticAnalysisImpl(const ConstParser& parser) const final;
};


struct TernaryNode final : public ExprNode {
  ~TernaryNode() final = default;

  TernaryNode(pExpr&& cond, pExpr&& _true, pExpr&& _false) :
      cond(std::move(cond)),
      _true(std::move(_true)),
      _false(std::move(_false)) {

  }

  pExpr cond;
  pExpr _true;
  pExpr _false;

  std::string ToString() const final {
    return cotyl::FormatStr("(%s) ? (%s) : (%s)", cond, _true->ToString(), _false);
  }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce(const Parser& parser) final;

protected:
  pType<const CType> SemanticAnalysisImpl(const ConstParser& parser) const final;
};


struct AssignmentNode final : public ExprNode {
  ~AssignmentNode() final = default;

  AssignmentNode(pExpr&& left, const TokenType op, pExpr&& right) :
      left(std::move(left)),
      op(op),
      right(std::move(right)) {

  }

  pExpr left;
  const TokenType op;
  pExpr right;

  std::string ToString() const final {
    return cotyl::FormatStr("%s %s (%s)", left, Token(op), right);
  }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pExpr EReduce(const Parser& parser) final;

protected:
  pType<const CType> SemanticAnalysisImpl(const ConstParser& parser) const final;
};

}