#pragma once

#include <memory>

namespace epi {

struct Node {

};


struct Expr : public Node {

};


template<typename T = Node>
using pNode = std::unique_ptr<T>;
using pExpr = pNode<Expr>;

template<typename T = Node, typename ...Args>
pNode<T> MakeNode(Args... args) {
  return std::make_unique<T>(args...);
}

}