#pragma once

#include "Default.h"
#include "types/Types.h"

#include <memory>
#include <string>


namespace epi {

struct Parser;

struct Node {
  virtual ~Node() = default;

  virtual std::string to_string() const = 0;
  virtual bool IsDeclaration() const { return false; }
};

struct Decl : public Node {

  bool IsDeclaration() const final { return true; }
};

struct Stat : public Node {

};

struct Expr : public Stat {

  // performs semantic analysis
  virtual pType<const CType> GetType(Parser&) const = 0;
  bool IsConstexpr(Parser& parser) const { return GetType(parser)->IsConstexpr(); }
  i64 ConstEval(Parser& parser) const { return GetType(parser)->ConstIntVal(); }
};

template<typename T = Node>
using pNode = std::unique_ptr<T>;
using pExpr = pNode<Expr>;

}