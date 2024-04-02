#pragma once

#include "Default.h"
#include "CustomAssert.h"
#include "types/EpiCType.h"

#include <memory>
#include <string>

namespace epi {
struct ConstParser;
struct Parser;
}

namespace epi::ast {

struct DeclarationNode;
struct FunctionDefinitionNode;

struct IdentifierNode;
template<typename T> struct NumericalConstantNode;
struct StringConstantNode;
struct ArrayAccessNode;
struct FunctionCallNode;
struct MemberAccessNode;
struct TypeInitializerNode;
struct PostFixNode;
struct UnopNode;
struct CastNode;
struct BinopNode;
struct TernaryNode;
struct AssignmentNode;

struct EmptyNode;
struct IfNode;
struct WhileNode;
struct DoWhileNode;
struct ForNode;
struct LabelNode;
struct SwitchNode;
struct CaseNode;
struct DefaultNode;
struct GotoNode;
struct ReturnNode;
struct BreakNode;
struct ContinueNode;
struct CompoundNode;

struct Node;
struct DeclNode;
struct StatNode;
struct ExprNode;

template<typename T = Node>
using pNode = std::unique_ptr<T>;
using pExpr = pNode<ExprNode>;

struct NodeVisitor;

struct Node {
  virtual ~Node() = default;

  virtual std::string ToString() const = 0;
  virtual pNode<> Reduce(const Parser& parser) = 0;
  virtual bool IsDeclarationNode() const { return false; }
  virtual bool IsStatement() const { return false; }

  virtual void Visit(NodeVisitor& visitor) = 0;
};

struct DeclNode : public Node {

  bool IsDeclarationNode() const final { return true; }
  virtual void VerifyAndRecord(Parser& parser) = 0;

  pNode<> Reduce(const Parser& parser) override { return nullptr; };
};

struct StatNode : public Node {

  bool IsStatement() const final { return true; }

  pNode<> Reduce(const Parser& parser) override { return SReduce(parser); }
  virtual pNode<StatNode> SReduce(const Parser& parser) { return nullptr; }
};

struct ExprNode : public StatNode {
  pType<const CType> type = nullptr;

  const pType<const CType>& GetType() const {
    cotyl::Assert(type != nullptr, "Semantic analysis has to be performed before type can be acquired");
    return type;
  }

  pType<const CType> SemanticAnalysis(const ConstParser& parser) {
    type = SemanticAnalysisImpl(parser);
    return type;
  }

protected:
  virtual pType<const CType> SemanticAnalysisImpl(const ConstParser&) const = 0;

public:
  bool IsConstexpr() const { return GetType()->IsConstexpr(); }
  i64 ConstEval() const { return GetType()->ConstIntVal(); }

  pNode<> Reduce(const Parser& parser) override { return EReduce(parser); }
  virtual pExpr EReduce(const Parser& parser);
  pNode<StatNode> SReduce(const Parser& parser) final { return EReduce(parser); }
};

}