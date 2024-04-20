#pragma once

#include "Node.h"
#include "Format.h"
#include "NodeVisitor.h"


namespace epi::ast {

struct DeclarationNode;

struct EmptyNode final : StatNode {
  std::string ToString() const final { return ";"; }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};

struct IfNode final : StatNode {

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
  pNode<> Reduce() final;
};

struct WhileNode final : StatNode {

  WhileNode(pExpr&& cond, pNode<StatNode>&& stat) :
      cond(std::move(cond)),
      stat(std::move(stat)) {

  }

  pExpr cond;
  pNode<StatNode> stat;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<> Reduce() final;
};

struct DoWhileNode final : StatNode {

  DoWhileNode(pNode<StatNode>&& stat, pExpr&& cond) :
      stat(std::move(stat)),
      cond(std::move(cond)) {

  }

  pNode<StatNode> stat;
  pExpr cond;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<> Reduce() final;
};

struct ForNode final : StatNode {

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
  pNode<> Reduce() final;
};

struct LabelNode final : StatNode {
  LabelNode(cotyl::CString&& name, pNode<StatNode> stat) :
      name(std::move(name)),
      stat(std::move(stat)) {

  }

  cotyl::CString name;
  pNode<StatNode> stat;

  std::string ToString() const final { return cotyl::FormatStr("%s: %s", name.str(), stat); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<> Reduce() final;
};


struct SwitchNode final : StatNode {

  SwitchNode(pExpr&& expr, pNode<StatNode>&& stat) :
          expr(std::move(expr)),
          stat(std::move(stat)) {

  }

  pExpr expr;
  pNode<StatNode> stat;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<> Reduce() final;
};

struct CaseNode final : StatNode {

  CaseNode(i64 expr, pNode<StatNode>&& stat) :
      expr(expr),
      stat(std::move(stat)) {

  }

  i64 expr;
  pNode<StatNode> stat;

  std::string ToString() const final { return cotyl::FormatStr("case %s: %s", expr, stat); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<> Reduce() final;
};

struct DefaultNode final : StatNode {
  DefaultNode(pNode<StatNode>&& stat) :
      stat(std::move(stat)) {

  }

  pNode<StatNode> stat;

  std::string ToString() const final { return cotyl::FormatStr("default: %s", stat); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<> Reduce() final;
};

struct GotoNode final : StatNode {

  GotoNode(cotyl::CString&& label) : label(std::move(label)) { }

  cotyl::CString label;

  std::string ToString() const final { return cotyl::Format("goto %s;", label.c_str()); }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};

struct ReturnNode final : StatNode {

  ReturnNode(pExpr expr = nullptr) : expr(std::move(expr)) {

  }

  pExpr expr;

  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<> Reduce() final;
};

struct BreakNode final : StatNode {

  std::string ToString() const final { return "break;"; }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};

struct ContinueNode final : StatNode {

  std::string ToString() const final { return "continue;"; }
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
};

struct CompoundNode final : StatNode {

  void AddNode(pNode<Node>&& stat) {
    if (stat) stats.push_back(std::move(stat));
  }

  std::vector<pNode<Node>> stats{};
  std::string ToString() const final;
  void Visit(NodeVisitor& visitor) final { visitor.Visit(*this); }
  pNode<> Reduce() final;
};

}