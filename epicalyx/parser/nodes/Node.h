#pragma once

#include <memory>
#include <string>


namespace epi {

struct Node {
  virtual ~Node() = default;

  virtual std::string to_string() const = 0;
};


struct Expr : public Node {
  virtual ~Expr() = default;

  virtual std::string to_string() const override = 0;
};


template<typename T = Node>
using pNode = std::unique_ptr<T>;
using pExpr = pNode<Expr>;

template<typename T = Node, typename ...Args>
pNode<T> MakeNode(Args... args) {
  return std::make_unique<T>(args...);
}

}