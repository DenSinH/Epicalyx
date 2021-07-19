#pragma once

#include "Node.h"
#include "types/Types.h"

#include <string>
#include <utility>
#include <vector>


namespace epi {

/*
 * PRIMARY EXPRESSION
 * */

struct Identifier : public Expr {

  Identifier(std::string name) :
      name(std::move(name)) {

  }

  const std::string name;
};


template<typename T>
struct NumericalConstant : public Expr {

  NumericalConstant(T value) :
      value(value) {

  }

  const T value;
};


struct StringConstant : public Expr {

  StringConstant(std::string value) :
      value(std::move(value)) {

  }

  const std::string value;
};

/*
 * POSTFIX EXPRESSION
 * */
struct ArrayAccess : public Expr {

  ArrayAccess(pExpr&& left, pExpr&& right) :
      left(std::move(left)),
      right(std::move(right)) {

  }

  const pExpr left, right;
};


struct FunctionCall : public Expr {

  FunctionCall(pExpr&& left) :
      left(std::move(left)) {

  }

  void AddArg(pExpr&& arg) {
    args.emplace_back(std::move(arg));
  }

  const pExpr left;
  std::vector<pExpr> args{};
};



struct MemberAccess : public Expr {
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
};


struct PostFix : public Expr {

  PostFix(pType<> (CType::*op)() const, pExpr&& left) :
      op(op),
      left(std::move(left)) {

  }

  const pExpr left;
  pType<> (CType::*const op)() const;
};

// todo: nicely with initializer lists
//struct Initializer : public Expr {
//
//
//  const pExpr type_name;
//  const pExpr
//};


struct Unary : public Expr {

  Unary(pType<> (CType::*op)() const, pExpr&& left) :
      op(op),
      left(std::move(left)) {

  }

  const pExpr left;
  pType<> (CType::*const op)() const;
};


struct Cast : public Expr {

  Cast(pNode<>&& type, pExpr&& expr) :
      type(std::move(type)),
      expr(std::move(expr)) {

  }

  // todo:
  const pNode<> type;
  const pExpr expr;
};


struct Binop : public Expr {

  Binop(pExpr&& left, pType<> (CType::*op)() const, pExpr&& right) :
      left(std::move(left)),
      op(op),
      right(std::move(right)) {

  }

  const pExpr left;
  pType<> (CType::*const op)() const;
  const pExpr right;
};


struct Ternary : public Expr {

  Ternary(pExpr&& cond, pExpr&& _true, pExpr&& _false) :
      cond(std::move(cond)),
      _true(std::move(_true)),
      _false(std::move(_false)) {

  }

  const pExpr cond;
  const pExpr _true;
  const pExpr _false;
};


struct Assignment : public Expr {

  Assignment(pExpr&& left, pType<> (CType::*op)(const CType& other), pExpr&& right) :
      left(std::move(left)),
      op(op),
      right(std::move(right)) {

  }

  const pExpr left;
  pType<> (CType::*const op)(const CType& other);
  const pExpr right;
};

}