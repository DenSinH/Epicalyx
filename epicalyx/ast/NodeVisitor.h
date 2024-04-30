#pragma once

#include "NodeFwd.h"


namespace epi::ast {

struct NodeVisitor {
  virtual void Visit(const DeclarationNode& decl) = 0;
  virtual void Visit(const FunctionDefinitionNode& decl) = 0;
  virtual void Visit(const IdentifierNode& decl) = 0;

  virtual void Visit(const NumericalConstantNode<i8>& expr) = 0;
  virtual void Visit(const NumericalConstantNode<u8>& expr) = 0;
  virtual void Visit(const NumericalConstantNode<i16>& expr) = 0;
  virtual void Visit(const NumericalConstantNode<u16>& expr) = 0;
  virtual void Visit(const NumericalConstantNode<i32>& expr) = 0;
  virtual void Visit(const NumericalConstantNode<u32>& expr) = 0;
  virtual void Visit(const NumericalConstantNode<i64>& expr) = 0;
  virtual void Visit(const NumericalConstantNode<u64>& expr) = 0;
  virtual void Visit(const NumericalConstantNode<float>& expr) = 0;
  virtual void Visit(const NumericalConstantNode<double>& expr) = 0;
  virtual void Visit(const StringConstantNode& expr) = 0;
  virtual void Visit(const ArrayAccessNode& expr) = 0;
  virtual void Visit(const FunctionCallNode& expr) = 0;
  virtual void Visit(const MemberAccessNode& expr) = 0;
  virtual void Visit(const TypeInitializerNode& expr) = 0;
  virtual void Visit(const PostFixNode& expr) = 0;
  virtual void Visit(const UnopNode& expr) = 0;
  virtual void Visit(const CastNode& expr) = 0;
  virtual void Visit(const BinopNode& expr) = 0;
  virtual void Visit(const TernaryNode& expr) = 0;
  virtual void Visit(const AssignmentNode& expr) = 0;
  virtual void Visit(const ExpressionListNode& expr) = 0;
  
  virtual void Visit(const EmptyNode& stat) = 0;
  virtual void Visit(const IfNode& stat) = 0;
  virtual void Visit(const WhileNode& stat) = 0;
  virtual void Visit(const DoWhileNode& stat) = 0;
  virtual void Visit(const ForNode& stat) = 0;
  virtual void Visit(const LabelNode& stat) = 0;
  virtual void Visit(const SwitchNode& stat) = 0;
  virtual void Visit(const CaseNode& stat) = 0;
  virtual void Visit(const DefaultNode& stat) = 0;
  virtual void Visit(const GotoNode& stat) = 0;
  virtual void Visit(const ReturnNode& stat) = 0;
  virtual void Visit(const BreakNode& stat) = 0;
  virtual void Visit(const ContinueNode& stat) = 0;
  virtual void Visit(const CompoundNode& stat) = 0;
};

#define VISIT_IMPL void Visit(NodeVisitor& visitor) const final { visitor.Visit(*this); }

}