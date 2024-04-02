#pragma once

#include "Node.h"
//#include "Expression.h"
#include "Format.h"
#include "NodeVisitor.h"


namespace epi::ast {

struct DeclarationNode;

struct EmptyNode : public StatNode {
  std::string ToString() const final { return ";"; }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};

struct IfNode : public StatNode {

  IfNode(pExpr&& cond, pNode<StatNode>&& stat, pNode<StatNode>&& _else = nullptr) :
      cond(std::move(cond)),
      stat(std::move(stat)),
      _else(std::move(_else)) {

  }

  pExpr cond;
  pNode<StatNode> stat;
  pNode<StatNode> _else;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<StatNode> SReduce(const Parser& parser) final;
};

struct WhileNode : public StatNode {

  WhileNode(pExpr&& cond, pNode<StatNode>&& stat) :
      cond(std::move(cond)),
      stat(std::move(stat)) {

  }

  pExpr cond;
  pNode<StatNode> stat;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<StatNode> SReduce(const Parser& parser) final;
};

struct DoWhileNode : public StatNode {

  DoWhileNode(pNode<StatNode>&& stat, pExpr&& cond) :
      stat(std::move(stat)),
      cond(std::move(cond)) {

  }

  pNode<StatNode> stat;
  pExpr cond;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<StatNode> SReduce(const Parser& parser) final;
};

struct ForNode : public StatNode {

  ForNode(
      std::vector<pNode<DeclarationNode>>&& decls,
      std::vector<pExpr>&& inits,
      pExpr&& cond,
      std::vector<pExpr>&& updates,
      pNode<StatNode>&& stat
  );


  std::vector<pNode<DeclarationNode>> decls{};
  std::vector<pExpr> inits{};
  pExpr cond;
  std::vector<pExpr> updates{};
  pNode<StatNode> stat;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<StatNode> SReduce(const Parser& parser) final;
};

struct LabelNode : public StatNode {
  LabelNode(std::string name, pNode<StatNode> stat) :
      name(std::move(name)),
      stat(std::move(stat)) {

  }

  const std::string name;
  pNode<StatNode> stat;

  std::string ToString() const final { return cotyl::FormatStr("%s: %s", name, stat); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<StatNode> SReduce(const Parser& parser) final;
};


struct SwitchNode : public StatNode {

  SwitchNode(pExpr&& expr, pNode<StatNode>&& stat) :
          expr(std::move(expr)),
          stat(std::move(stat)) {

  }

  pExpr expr;
  pNode<StatNode> stat;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<StatNode> SReduce(const Parser& parser) final;
};

struct CaseNode : public StatNode {

  CaseNode(i64 expr, pNode<StatNode>&& stat) :
      expr(expr),
      stat(std::move(stat)) {

  }

  i64 expr;
  pNode<StatNode> stat;

  std::string ToString() const final { return cotyl::FormatStr("case %s: %s", expr, stat); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<StatNode> SReduce(const Parser& parser) final;
};

struct DefaultNode : public StatNode {
  DefaultNode(pNode<StatNode>&& stat) :
      stat(std::move(stat)) {

  }

  pNode<StatNode> stat;

  std::string ToString() const final { return cotyl::FormatStr("default: %s", stat); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<StatNode> SReduce(const Parser& parser) final;
};

struct GotoNode : public StatNode {

  GotoNode(std::string label) : label(std::move(label)) { }

  const std::string label;

  std::string ToString() const final { return cotyl::FormatStr("goto %s;", label); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};

struct ReturnNode : public StatNode {

  ReturnNode(pExpr expr = nullptr) : expr(std::move(expr)) {

  }

  pExpr expr;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<StatNode> SReduce(const Parser& parser) final;
};

struct BreakNode : public StatNode {

  std::string ToString() const final { return "break;"; }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};

struct ContinueNode : public StatNode {

  std::string ToString() const final { return "continue;"; }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};

struct CompoundNode : public StatNode {

  void AddNode(pNode<Node>&& stat) {
    if (stat) stats.push_back(std::move(stat));
  }

  std::vector<pNode<Node>> stats{};
  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<StatNode> SReduce(const Parser& parser) final;
};

}