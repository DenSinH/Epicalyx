#ifndef EPICALYX_EXPRESSION_H
#define EPICALYX_EXPRESSION_H

#include <memory>
#include <utility>
#include "../AST.h"

class UnaryExpression;
class BinOpExpression;
class AssignmentExpression;

class Expression : public Node {
public:
    Expression() {
        this->Left = nullptr;
        this->Right = nullptr;
    }

    Expression(std::unique_ptr<Expression> left, std::unique_ptr<AssignmentExpression> right) {
        this->Left = std::move(left);
        this->Right = std::move(right);
    }

    std::unique_ptr<Expression> Left;
    std::unique_ptr<AssignmentExpression> Right;
};

// Expression can just be an AssignmentExpression
class AssignmentExpression : public Expression {
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

    AssignmentExpression() {
        this->Left = nullptr;
        this->Op = AssignOp::Eq;
        this->Right = nullptr;
    }

    AssignmentExpression(
            std::unique_ptr<UnaryExpression> left,
            AssignOp op,
            std::unique_ptr<BinOpExpression> right) {
        this->Left = std::move(left);
        this->Op = op;
        this->Right = std::move(right);
    }

    std::unique_ptr<UnaryExpression> Left;
    AssignOp Op;
    std::unique_ptr<BinOpExpression> Right;  // can also be a CondExpr
};

#endif //EPICALYX_EXPRESSION_H
