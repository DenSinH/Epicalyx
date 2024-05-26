#pragma once

#include <string>           // for string
#include <utility>          // for move
#include "CString.h"        // for stringify
#include "Default.h"        // for i64
#include "Exceptions.h"     // for Exception
#include "NodeFwd.h"        // for pStat
#include "types/AnyType.h"  // for AnyType

namespace epi { namespace ast { struct NodeVisitor; } }

namespace epi::ast {
using ::epi::stringify;

struct ASTError : cotyl::Exception {
  ASTError(std::string&& message) : 
      Exception("Bad AST", std::move(message)) { }
};

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