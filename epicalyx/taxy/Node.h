#pragma once

#include "Default.h"
#include "Assert.h"
#include "types/EpiCType.h"

#include <memory>
#include <string>

namespace epi {
struct Parser;
}

namespace epi::taxy {

struct Declaration;
struct FunctionDefinition;

struct Identifier;
template<typename T> struct NumericalConstant;
struct StringConstant;
struct ArrayAccess;
struct FunctionCall;
struct MemberAccess;
struct TypeInitializer;
struct PostFix;
struct Unary;
struct Cast;
struct Binop;
struct Ternary;
struct Assignment;

struct Empty;
struct If;
struct While;
struct DoWhile;
struct For;
struct Label;
struct Switch;
struct Case;
struct Default;
struct Goto;
struct Return;
struct Break;
struct Continue;
struct Compound;

struct Node;
struct Decl;
struct Stat;
struct Expr;

template<typename T = Node>
using pNode = std::unique_ptr<T>;
using pExpr = pNode<Expr>;

struct NodeVisitor;

struct Node {
  virtual ~Node() = default;

  virtual std::string ToString() const = 0;
  virtual pNode<> Reduce(const Parser& parser) = 0;
  virtual bool IsDeclaration() const { return false; }
  virtual bool IsStatement() const { return false; }

  virtual void Visit(NodeVisitor& visitor) = 0;
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
  pType<const CType> type = nullptr;

  const pType<const CType>& GetType() const {
    cotyl::Assert(type != nullptr, "Semantic analysis has to be performed before type can be acquired");
    return type;
  }

  pType<const CType> SemanticAnalysis(const Parser& parser) {
    type = SemanticAnalysisImpl(parser);
    return type;
  }

protected:
  virtual pType<const CType> SemanticAnalysisImpl(const Parser&) const = 0;

public:
  bool IsConstexpr() const { return GetType()->IsConstexpr(); }
  i64 ConstEval() const { return GetType()->ConstIntVal(); }

  pNode<> Reduce(const Parser& parser) override { return EReduce(parser); }
  virtual pExpr EReduce(const Parser& parser);
  pNode<Stat> SReduce(const Parser& parser) final { return EReduce(parser); }
};

}