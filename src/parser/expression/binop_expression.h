#ifndef EPICALYX_BINOP_EXPRESSION_H
#define EPICALYX_BINOP_EXPRESSION_H

#include <memory>
#include <utility>
#include "../AST.h"
#include "expression.h"

class Expression;
class BinOpExpression;
class CastExpression;

// AssignmentExpression can just be a CondExpr
class CondExpr : public AssignmentExpression {
public:
    CondExpr() {
        this->Left = nullptr;
        this->True = nullptr;
        this->False = nullptr;
    }

    CondExpr(std::unique_ptr<BinOpExpression> left,
             std::unique_ptr<Expression> t,
             std::unique_ptr<CondExpr> f) {
        this->Left = std::move(left);
        this->True = std::move(t);
        this->False = std::move(f);
    }

    std::unique_ptr<BinOpExpression> Left;
    std::unique_ptr<Expression> True;
    std::unique_ptr<CondExpr> False;
};

// condition expression can just be a binop expression
class BinOpExpression : public CondExpr {
public:
    enum class BinOp {
        Mul,
        Div,
        Mod,
        Add,
        Sub,
        LShift,
        RShift,
        Leq,
        Geq,
        Le,
        Ge,
        Eq,
        Ne,
        BinAnd,
        BinXOr,
        BinOr,
        LogicAnd,
        LogicOr,
        NONE,
    };

    BinOpExpression() {
        this->Op = BinOp::NONE;
        this->Left = nullptr;
        this->Right = nullptr;
    }

    BinOpExpression(BinOp op, std::unique_ptr<BinOpExpression> left, std::unique_ptr<CastExpression> right) {
        this->Op = op;
        this->Left = std::move(left);
        this->Right = std::move(right);
    }

    BinOp Op;
    std::unique_ptr<BinOpExpression> Left;
    std::unique_ptr<CastExpression> Right;
};

#endif //EPICALYX_BINOP_EXPRESSION_H
