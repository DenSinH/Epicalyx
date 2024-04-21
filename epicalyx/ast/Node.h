#pragma once

#include "Default.h"
#include "NodeFwd.h"
#include "CustomAssert.h"
#include "types/AnyType.h"

#include <memory>
#include <string>
#include <type_traits>

namespace epi::ast {
using ::epi::stringify;

struct Node {
  virtual ~Node() = default;

  virtual std::string ToString() const = 0;
  virtual void Visit(NodeVisitor& visitor) = 0;
};

struct DeclNode : public Node {

};

struct StatNode : public Node {
  virtual pStat Reduce() { return nullptr; };
};

struct ExprNode : public StatNode {
  type::AnyType type;

  ExprNode(type::AnyType type) : type{std::move(type)} { }
  
  bool IsConstexpr() const { return type.IsConstexpr(); }
  bool ConstEval() const { return type.ConstBoolVal(); }
  i64 ConstIntVal() const { return type.ConstIntVal(); }
};

}