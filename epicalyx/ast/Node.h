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
  virtual void Visit(NodeVisitor& visitor) const = 0;
};

struct DeclNode : public Node {

};

struct StatNode : public Node {
  virtual pStat Reduce() { return nullptr; };
};

struct ExprNode : public StatNode {
  type::AnyType type;

  ExprNode(type::AnyType type) : type{std::move(type)} { }
  
  void VerifySwitchable() const;
  void VerifyTruthiness() const;
  bool IsConstexpr() const;
  i64 ConstIntVal() const;
  bool ConstBoolVal() const;
};

}