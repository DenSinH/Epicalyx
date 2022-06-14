#pragma once

#include "Node.h"


namespace epi::ast {

struct NodeVisitor {
  virtual void Visit(Declaration& decl) = 0;
  virtual void Visit(FunctionDefinition& decl) = 0;
  virtual void Visit(Identifier& decl) = 0;

  virtual void Visit(NumericalConstant<i8>& expr) = 0;
  virtual void Visit(NumericalConstant<u8>& expr) = 0;
  virtual void Visit(NumericalConstant<i16>& expr) = 0;
  virtual void Visit(NumericalConstant<u16>& expr) = 0;
  virtual void Visit(NumericalConstant<i32>& expr) = 0;
  virtual void Visit(NumericalConstant<u32>& expr) = 0;
  virtual void Visit(NumericalConstant<i64>& expr) = 0;
  virtual void Visit(NumericalConstant<u64>& expr) = 0;
  virtual void Visit(NumericalConstant<float>& expr) = 0;
  virtual void Visit(NumericalConstant<double>& expr) = 0;
  virtual void Visit(StringConstant& expr) = 0;
  virtual void Visit(ArrayAccess& expr) = 0;
  virtual void Visit(FunctionCall& expr) = 0;
  virtual void Visit(MemberAccess& expr) = 0;
  virtual void Visit(TypeInitializer& expr) = 0;
  virtual void Visit(PostFix& expr) = 0;
  virtual void Visit(Unary& expr) = 0;
  virtual void Visit(Cast& expr) = 0;
  virtual void Visit(Binop& expr) = 0;
  virtual void Visit(Ternary& expr) = 0;
  virtual void Visit(Assignment& expr) = 0;

  virtual void Visit(Empty& stat) = 0;
  virtual void Visit(If& stat) = 0;
  virtual void Visit(While& stat) = 0;
  virtual void Visit(DoWhile& stat) = 0;
  virtual void Visit(For& stat) = 0;
  virtual void Visit(Label& stat) = 0;
  virtual void Visit(Switch& stat) = 0;
  virtual void Visit(Case& stat) = 0;
  virtual void Visit(Default& stat) = 0;
  virtual void Visit(Goto& stat) = 0;
  virtual void Visit(Return& stat) = 0;
  virtual void Visit(Break& stat) = 0;
  virtual void Visit(Continue& stat) = 0;
  virtual void Visit(Compound& stat) = 0;
};

}