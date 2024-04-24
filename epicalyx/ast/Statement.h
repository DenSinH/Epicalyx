#pragma once

#include "Node.h"
#include "NodeVisitor.h"


namespace epi::ast {

struct EmptyNode final : StatNode {
  std::string ToString() const final { return ""; }
  VISIT_IMPL
};

struct IfNode final : StatNode {

  IfNode(pExpr&& cond, pStat&& stat, pStat&& _else = nullptr);

  pExpr cond;
  pStat stat;
  pStat _else;

  std::string ToString() const final;
  VISIT_IMPL
  pStat Reduce() final;
};

struct WhileNode final : StatNode {

  WhileNode(pExpr&& cond, pStat&& stat);

  pExpr cond;
  pStat stat;

  std::string ToString() const final;
  VISIT_IMPL
  pStat Reduce() final;
};

struct DoWhileNode final : StatNode {

  DoWhileNode(pStat&& stat, pExpr&& cond);

  pStat stat;
  pExpr cond;

  std::string ToString() const final;
  VISIT_IMPL
  pStat Reduce() final;
};

struct ForNode final : StatNode {

  ForNode(
      cotyl::vector<DeclarationNode>&& decls,
      cotyl::vector<pExpr>&& inits,
      pExpr&& cond,
      cotyl::vector<pExpr>&& updates,
      pStat&& stat
  );


  cotyl::vector<DeclarationNode> decls{};
  cotyl::vector<pExpr> inits{};
  pExpr cond;
  cotyl::vector<pExpr> updates{};
  pStat stat;

  std::string ToString() const final;
  VISIT_IMPL
  pStat Reduce() final;
};

struct LabelNode final : StatNode {

  LabelNode(cotyl::CString&& name, pStat stat);

  cotyl::CString name;
  pStat stat;

  std::string ToString() const final;
  VISIT_IMPL
};


struct SwitchNode final : StatNode {

  SwitchNode(pExpr&& expr, pStat&& stat);

  pExpr expr;
  pStat stat;

  std::string ToString() const final;
  VISIT_IMPL
};

struct CaseNode final : StatNode {

  CaseNode(i64 expr, pStat&& stat);

  i64 expr;
  pStat stat;

  std::string ToString() const final;
  VISIT_IMPL
};

struct DefaultNode final : StatNode {

  DefaultNode(pStat&& stat);

  pStat stat;

  std::string ToString() const final;
  VISIT_IMPL
};

struct GotoNode final : StatNode {

  GotoNode(cotyl::CString&& label);

  cotyl::CString label;

  std::string ToString() const final;
  VISIT_IMPL
};

struct ReturnNode final : StatNode {

  ReturnNode(pExpr expr = nullptr);

  pExpr expr;

  std::string ToString() const final;
  VISIT_IMPL
};

struct BreakNode final : StatNode {

  std::string ToString() const final;
  VISIT_IMPL
};

struct ContinueNode final : StatNode {

  std::string ToString() const;
  VISIT_IMPL
};

struct CompoundNode final : StatNode {

  void AddNode(pNode<Node>&& stat);

  cotyl::vector<pNode<Node>> stats{};
  std::string ToString() const final;
  VISIT_IMPL
};

}