#pragma once

#include "Node.h"
#include "Expression.h"
#include "Format.h"
#include "Declaration.h"


namespace epi {

struct Declaration;

struct Empty : public Stat {
  std::string to_string() const final { return ";"; }
};

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
  pNode<Stat> SReduce(const Parser& parser) final;
};

struct While : public Stat {

  While(pExpr&& cond, pNode<Stat>&& stat) :
      cond(std::move(cond)),
      stat(std::move(stat)) {

  }

  pExpr cond;
  pNode<Stat> stat;

  std::string to_string() const final;
  pNode<Stat> SReduce(const Parser& parser) final;
};

struct DoWhile : public Stat {

  DoWhile(pNode<Stat>&& stat, pExpr&& cond) :
      stat(std::move(stat)),
      cond(std::move(cond)) {

  }

  pNode<Stat> stat;
  pExpr cond;

  std::string to_string() const final;
  pNode<Stat> SReduce(const Parser& parser) final;
};

struct For : public Stat {

  For(std::vector<pNode<InitDeclaration>>&& decls,
      std::vector<pExpr>&& inits,
      pExpr&& cond,
      std::vector<pExpr>&& updates,
      pNode<Stat>&& stat) :
          decls(std::move(decls)),
          inits(std::move(inits)),
          cond(std::move(cond)),
          updates(std::move(updates)),
          stat(std::move(stat)) {

  }


  std::vector<pNode<InitDeclaration>> decls{};
  std::vector<pExpr> inits{};
  pExpr cond;
  std::vector<pExpr> updates{};
  pNode<Stat> stat;

  std::string to_string() const final;
  pNode<Stat> SReduce(const Parser& parser) final;
};

struct Label : public Stat {
  Label(std::string name, pNode<Stat> stat) :
      name(std::move(name)),
      stat(std::move(stat)) {

  }

  const std::string name;
  pNode<Stat> stat;

  std::string to_string() const final { return cotyl::FormatStr("%s: %s", name, stat); }
  pNode<Stat> SReduce(const Parser& parser) final;
};


struct Switch : public Stat {

  Switch(pExpr&& expr, pNode<Stat>&& stat) :
          expr(std::move(expr)),
          stat(std::move(stat)) {

  }

  pExpr expr;
  pNode<Stat> stat;

  std::string to_string() const final;
  pNode<Stat> SReduce(const Parser& parser) final;
};

struct Case : public Stat {

  Case(i64 expr, pNode<Stat>&& stat) :
      expr(expr),
      stat(std::move(stat)) {

  }

  i64 expr;
  pNode<Stat> stat;

  std::string to_string() const final { return cotyl::FormatStr("case %s: %s", expr, stat); }
  pNode<Stat> SReduce(const Parser& parser) final;
};

struct Default : public Stat {
  Default(pNode<Stat>&& stat) :
      stat(std::move(stat)) {

  }

  pNode<Stat> stat;

  std::string to_string() const final { return cotyl::FormatStr("default: %s", stat); }
  pNode<Stat> SReduce(const Parser& parser) final;
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
  pNode<Stat> SReduce(const Parser& parser) final;
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
  pNode<Stat> SReduce(const Parser& parser) final;
};

}