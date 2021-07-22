#pragma once

#include "Node.h"
#include "Expression.h"
#include "Format.h"
#include "Declaration.h"


namespace epi {

struct Declaration;

struct If : public Stat {

  If(pExpr&& cond, pNode<Stat>&& stat, pNode<Stat>&& _else = nullptr) :
      cond(std::move(cond)),
      stat(std::move(stat)),
      _else(std::move(_else)) {

  }

  pExpr cond;
  pNode<Stat> stat;
  pNode<Stat> _else;

  std::string to_string() const final;
};

struct While : public Stat {

  While(pExpr&& cond, pNode<Stat>&& stat) :
      cond(std::move(cond)),
      stat(std::move(stat)) {

  }

  pExpr cond;
  pNode<Stat> stat;

  std::string to_string() const final;
};

struct DoWhile : public Stat {

  DoWhile(pNode<Stat>&& stat, pExpr&& cond) :
      stat(std::move(stat)),
      cond(std::move(cond)) {

  }

  pNode<Stat> stat;
  pExpr cond;

  std::string to_string() const final;
};

struct For : public Stat {

  For(std::vector<pNode<Declaration>>&& decl,
      std::vector<pExpr>&& init,
      pExpr&& cond,
      std::vector<pExpr>&& update,
      pNode<Stat>&& stat) :
          decl(std::move(decl)),
          init(std::move(init)),
          cond(std::move(cond)),
          update(std::move(update)),
          stat(std::move(stat)) {

  }


  std::vector<pNode<Declaration>> decl{};
  std::vector<pExpr> init{};
  pExpr cond;
  std::vector<pExpr> update{};
  pNode<Stat> stat;

  std::string to_string() const final;
};

struct Label : public Stat {
  Label(std::string name, pNode<Stat> stat) :
      name(std::move(name)),
      stat(std::move(stat)) {

  }

  const std::string name;
  pNode<Stat> stat;

  std::string to_string() const final { return cotyl::FormatStr("%s: %s", name, stat); };
};

struct Case : public Stat {

  Case(pExpr&& expr, pNode<Stat>&& stat) :
      expr(std::move(expr)),
      stat(std::move(stat)) {

  }

  pExpr expr;
  pNode<Stat> stat;

  std::string to_string() const final { return cotyl::FormatStr("case %s: %s", expr, stat); }
};

struct Default : public Stat {
  Default(pNode<Stat>&& stat) :
      stat(std::move(stat)) {

  }

  pNode<Stat> stat;

  std::string to_string() const final { return cotyl::FormatStr("default: %s", stat); }
};

struct Goto : public Stat {

  Goto(std::string label) : label(std::move(label)) { }

  const std::string label;

  std::string to_string() const final { return cotyl::FormatStr("goto %s;", label); }
};

struct Return : public Stat {

  Return(pExpr expr = nullptr) : expr(std::move(expr)) {

  }

  pExpr expr;

  std::string to_string() const final;
};

struct Break : public Stat {

  std::string to_string() const final { return "break;"; }
};

struct Continue : public Stat {

  std::string to_string() const final { return "continue;"; }
};

struct Compound : public Stat {

  void AddNode(pNode<Node>&& stat) {
    if (stat) stats.push_back(std::move(stat));
  }

  std::vector<pNode<Node>> stats{};
  std::string to_string() const final;
};

}