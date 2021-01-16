#ifndef EPICALYX_BINOP_EXPRESSION_H
#define EPICALYX_BINOP_EXPRESSION_H

#include <memory>
#include <utility>
#include "../AST.h"

class CondExpr : public Expr {
public:
    CondExpr(std::unique_ptr<Expr> left,
             std::unique_ptr<Expr> t,
             std::unique_ptr<Expr> f) {
        this->Left = std::move(left);
        this->True = std::move(t);
        this->False = std::move(f);
    }

    std::unique_ptr<Expr> Left;
    std::unique_ptr<Expr> True;
    std::unique_ptr<Expr> False;
};


class BinOpExpression : public Expr {
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
    };

    BinOpExpression(BinOp op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right) {
        this->Op = op;
        this->Left = std::move(left);
        this->Right = std::move(right);
    }

    BinOp Op;
    std::unique_ptr<Expr> Left;
    std::unique_ptr<Expr> Right;
};

#endif //EPICALYX_BINOP_EXPRESSION_H
