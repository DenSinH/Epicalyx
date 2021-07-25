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

  virtual pType<const CType> GetType(Parser&) { return MakeType<VoidType>(); };  // performs semantic analysis
  virtual bool IsConstexpr() const { return false; }
  virtual i64 ConstEval() const { return 0; }
};

template<typename T = Node>
using pNode = std::unique_ptr<T>;
using pExpr = pNode<Expr>;

}