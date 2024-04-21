#pragma once

#include "NodeFwd.h"


namespace epi::ast {

struct NodeVisitor {
  virtual void Visit(DeclarationNode& decl) = 0;
  virtual void Visit(FunctionDefinitionNode& decl) = 0;
  virtual void Visit(IdentifierNode& decl) = 0;

  virtual void Visit(NumericalConstantNode<i8>& expr) = 0;
  virtual void Visit(NumericalConstantNode<u8>& expr) = 0;
  virtual void Visit(NumericalConstantNode<i16>& expr) = 0;
  virtual void Visit(NumericalConstantNode<u16>& expr) = 0;
  virtual void Visit(NumericalConstantNode<i32>& expr) = 0;
  virtual void Visit(NumericalConstantNode<u32>& expr) = 0;
  virtual void Visit(NumericalConstantNode<i64>& expr) = 0;
  virtual void Visit(NumericalConstantNode<u64>& expr) = 0;
  virtual void Visit(NumericalConstantNode<float>& expr) = 0;
  virtual void Visit(NumericalConstantNode<double>& expr) = 0;
  virtual void Visit(StringConstantNode& expr) = 0;
  virtual void Visit(ArrayAccessNode& expr) = 0;
  virtual void Visit(FunctionCallNode& expr) = 0;
  virtual void Visit(MemberAccessNode& expr) = 0;
  virtual void Visit(TypeInitializerNode& expr) = 0;
  virtual void Visit(PostFixNode& expr) = 0;
  virtual void Visit(UnopNode& expr) = 0;
  virtual void Visit(CastNode& expr) = 0;
  virtual void Visit(BinopNode& expr) = 0;
  virtual void Visit(TernaryNode& expr) = 0;
  virtual void Visit(AssignmentNode& expr) = 0;

  virtual void Visit(EmptyNode& stat) = 0;
  virtual void Visit(IfNode& stat) = 0;
  virtual void Visit(WhileNode& stat) = 0;
  virtual void Visit(DoWhileNode& stat) = 0;
  virtual void Visit(ForNode& stat) = 0;
  virtual void Visit(LabelNode& stat) = 0;
  virtual void Visit(SwitchNode& stat) = 0;
  virtual void Visit(CaseNode& stat) = 0;
  virtual void Visit(DefaultNode& stat) = 0;
  virtual void Visit(GotoNode& stat) = 0;
  virtual void Visit(ReturnNode& stat) = 0;
  virtual void Visit(BreakNode& stat) = 0;
  virtual void Visit(ContinueNode& stat) = 0;
  virtual void Visit(CompoundNode& stat) = 0;
};

}