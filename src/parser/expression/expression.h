#ifndef EPICALYX_EXPRESSION_H
#define EPICALYX_EXPRESSION_H

#include <memory>
#include <utility>
#include "../AST.h"

class UnaryExpression;
class BinOpExpression;
class AssignmentExpression;

class Expression : public Expr {
public:
    Expression(std::unique_ptr<Expr> left, std::unique_ptr<Expr> right) {
        this->Left = std::move(left);
        this->Right = std::move(right);
    }

    std::unique_ptr<Expr> Left;
    std::unique_ptr<Expr> Right;
};

class AssignmentExpression : public Expr {
public:
    enum class AssignOp {
        Eq,
        MulEq,
        DivEq,
        ModEq,
        AddEq,
        SubEq,
        LShiftEq,
        RShiftEq,
        AndEq,
        XorEq,
        OrEq,
    };

    AssignmentExpression(
            std::unique_ptr<Expr> left,
            AssignOp op,
            std::unique_ptr<Expr> right) {
        this->Left = std::move(left);
        this->Op = op;
        this->Right = std::move(right);
    }

    std::unique_ptr<Expr> Left;
    AssignOp Op;
    std::unique_ptr<Expr> Right;  // can also be a CondExpr
};

#endif //EPICALYX_EXPRESSION_H
