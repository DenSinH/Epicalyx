#pragma once

#include "Node.h"
#include "types/Types.h"
#include "tokenizer/Token.h"

#include <string>
#include <utility>
#include <vector>


namespace epi {

/*
 * PRIMARY EXPRESSION
 * */

struct Identifier final : public Expr {
  ~Identifier() final = default;

  Identifier(std::string name) :
      name(std::move(name)) {

  }

  const std::string name;

  std::string to_string() const final { return name; };
};


template<typename T>
struct NumericalConstant final : public Expr {
  ~NumericalConstant() final = default;

  NumericalConstant(T value) :
      value(value) {

  }

  const T value;

  std::string to_string() const final { return std::to_string(value); };
};


struct StringConstant final : public Expr {
  ~StringConstant() final = default;

  StringConstant(std::string value) :
      value(std::move(value)) {

  }

  const std::string value;

  std::string to_string() const final { return cotyl::FormatStr("\"%s\"", value); }
};

/*
 * POSTFIX EXPRESSION
 * */
struct ArrayAccess final : public Expr {
  ~ArrayAccess() final = default;

  ArrayAccess(pExpr&& left, pExpr&& right) :
      left(std::move(left)),
      right(std::move(right)) {

  }

  const pExpr left, right;

  std::string to_string() const final { return cotyl::FormatStr("(%s)[%s]", left->to_string(), right->to_string()); }
};


struct FunctionCall final : public Expr {
  ~FunctionCall() final = default;

  FunctionCall(pExpr&& left) :
      left(std::move(left)) {

  }

  void AddArg(pExpr&& arg) {
    args.emplace_back(std::move(arg));
  }

  const pExpr left;
  std::vector<pExpr> args{};

  std::string to_string() const final;
};


struct MemberAccess final : public Expr {
  ~MemberAccess() final = default;

  MemberAccess(pExpr&& left, std::string member) :
      left(std::move(left)),
      member(std::move(member)) {

  }
  
  static pNode<MemberAccess> Arrow(pExpr&& left, std::string member) {
    // todo:
    return nullptr; // MakeExpr<MemberAccess>()
  }

  const pExpr left;
  const std::string member;

  std::string to_string() const final { return cotyl::FormatStr("(%s).%s", left->to_string(), member); }
};


struct PostFix final : public Expr {
  ~PostFix() final = default;

  PostFix(const TokenType op, pExpr&& left) :
      op(op),
      left(std::move(left)) {

  }

  const pExpr left;
  const TokenType op;

  std::string to_string() const final { return cotyl::FormatStr("(%s)%s", left->to_string(), Token(op).to_string()); }
};

// todo: nicely with initializer lists
//struct Initializer final : public Expr {
//
//
//  const pExpr type_name;
//  const pExpr
//};


struct Unary final : public Expr {
  ~Unary() final = default;

  Unary(TokenType op, pExpr&& left) :
      op(op),
      left(std::move(left)) {

  }

  const pExpr left;
  const TokenType op;

  std::string to_string() const final { return cotyl::FormatStr("%s(%s)", Token(op).to_string(), left->to_string()); }
};


struct Cast final : public Expr {
  ~Cast() final = default;

  Cast(pNode<>&& type, pExpr&& expr) :
      type(std::move(type)),
      expr(std::move(expr)) {

  }

  // todo:
  const pNode<> type;
  const pExpr expr;

  std::string to_string() const final { return cotyl::FormatStr("(%s)(%s)", type->to_string(), expr->to_string()); }
};


struct Binop final : public Expr {
  ~Binop() final = default;

  Binop(pExpr&& left, const TokenType op, pExpr&& right) :
      left(std::move(left)),
      op(op),
      right(std::move(right)) {

  }

  const pExpr left;
  const TokenType op;
  const pExpr right;

  std::string to_string() const final {
    return cotyl::FormatStr("(%s) %s (%s)", left->to_string(), Token(op).to_string(), right->to_string());
  }
};


struct Ternary final : public Expr {
  ~Ternary() final = default;

  Ternary(pExpr&& cond, pExpr&& _true, pExpr&& _false) :
      cond(std::move(cond)),
      _true(std::move(_true)),
      _false(std::move(_false)) {

  }

  const pExpr cond;
  const pExpr _true;
  const pExpr _false;

  std::string to_string() const final {
    return cotyl::FormatStr("(%s) ? (%s) : (%s)", cond->to_string(), _true->to_string(), _false->to_string());
  }
};


struct Assignment final : public Expr {
  ~Assignment() final = default;

  Assignment(pExpr&& left, const TokenType op, pExpr&& right) :
      left(std::move(left)),
      op(op),
      right(std::move(right)) {

  }

  const pExpr left;
  const TokenType op;
  const pExpr right;

  std::string to_string() const final {
    return cotyl::FormatStr("(%s) %s (%s)", left->to_string(), Token(op).to_string(), right->to_string());
  }
};

}