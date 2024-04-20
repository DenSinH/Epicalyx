#pragma once

#include "Default.h"
#include "CustomAssert.h"
#include "types/AnyType.h"

#include <memory>
#include <string>

namespace epi::ast {
using ::epi::stringify;

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
  virtual pNode<> Reduce() { return nullptr; };
  virtual void Visit(NodeVisitor& visitor) = 0;
};

struct DeclNode : public Node {


};

struct StatNode : public Node {

};

struct ExprNode : public StatNode {
  type::AnyType type;

  ExprNode(type::AnyType type) : type{std::move(type)} { }

  virtual pExpr EReduce() { return nullptr; };
  
  pNode<> Reduce() final;
public:
  bool IsConstexpr() const { return type.IsConstexpr(); }
  i64 ConstEval() const { return type.ConstIntVal(); }
};

}