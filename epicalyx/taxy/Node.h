#pragma once

#include "Default.h"
#include "types/EpiCType.h"

#include <memory>
#include <string>

namespace epi {
struct Parser;
}

namespace epi::taxy {

struct Expr;
struct Node;

template<typename T = Node>
using pNode = std::unique_ptr<T>;
using pExpr = pNode<Expr>;

struct Node {
  virtual ~Node() = default;

  virtual std::string ToString() const = 0;
  virtual pNode<> Reduce(const Parser& parser) = 0;
  virtual bool IsDeclaration() const { return false; }
  virtual bool IsStatement() const { return false; }
};

struct Decl : public Node {

  bool IsDeclaration() const final { return true; }
  virtual void VerifyAndRecord(Parser& parser) = 0;

  pNode<> Reduce(const Parser& parser) override { return nullptr; };
};

struct Stat : public Node {

  bool IsStatement() const final { return true; }

  pNode<> Reduce(const Parser& parser) override { return SReduce(parser); }
  virtual pNode<Stat> SReduce(const Parser& parser) { return nullptr; }
};

struct Expr : public Stat {

  // performs semantic analysis
  virtual pType<const CType> GetType(const Parser&) const = 0;

  bool IsConstexpr(const Parser& parser) const { return GetType(parser)->IsConstexpr(); }
  i64 ConstEval(const Parser& parser) const { return GetType(parser)->ConstIntVal(); }
  void Validate(const Parser& parser) const { GetType(parser); };

  pNode<> Reduce(const Parser& parser) override { return EReduce(parser); }
  virtual pExpr EReduce(const Parser& parser);
  pNode<Stat> SReduce(const Parser& parser) final { return EReduce(parser); }
};

}