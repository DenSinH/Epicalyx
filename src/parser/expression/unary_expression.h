#ifndef EPICALYX_UNARY_EXPRESSION_H
#define EPICALYX_UNARY_EXPRESSION_H


#include <memory>
#include <utility>
#include <vector>
#include "../AST.h"


// a cast expression can just be a unary expression
class UnaryExpression : public Expr {
public:
    enum class UnExprType {
        PostFix,
        UnOp,  // ++/--/&/*/+/-/~/!
        SizeOf,
    };

    explicit UnaryExpression(UnExprType type) {
        this->Type = type;
    }

    UnExprType Type;
};

class UnaryOpExpression : public UnaryExpression {
public:
    enum class UnOpType {
        PreIncrement,
        PreDecrement,
        Reference,
        Dereference,
        Positive,
        Negative,
        BinaryNot,
        LogicalNot,
    };

    explicit UnaryOpExpression(UnOpType type, std::unique_ptr<Expr> right) : UnaryExpression(UnExprType::UnOp) {
        this->Type = type;
        this->Right = std::move(right);
    }

    UnOpType Type;
    std::unique_ptr<Expr> Right;
};

class SizeOfExpression : public UnaryExpression {

    explicit SizeOfExpression(std::unique_ptr<Expr> right) : UnaryExpression(UnExprType::SizeOf) {
        this->Right = std::move(right);
    }

    std::unique_ptr<Expr> Right;
};

#endif //EPICALYX_UNARY_EXPRESSION_H
