#ifndef EPICALYX_UNARY_EXPRESSION_H
#define EPICALYX_UNARY_EXPRESSION_H


#include <memory>
#include <utility>
#include <vector>
#include "../AST.h"
#include "cast_expression.h"


class UnaryExpression : public CastExpression {
public:
    enum class UnExprType {
        PostFix,
        UnOp,  // ++/--/&/*/+/-/~/!
        SizeOf,
    };

    explicit UnaryExpression(UnExprType type) : CastExpression() {
        this->Type = type;
    }

    UnExprType Type;
};

class UnaryOpExpression : public UnaryExpression {
public:
    enum class UnOpType {
        Increment,
        Decrement,
        Reference,
        Dereference,
        Positive,
        Negative,
        BinaryNot,
        LogicalNot,
    };

    explicit UnaryOpExpression(UnOpType type, std::unique_ptr<CastExpression> expr) : UnaryExpression(UnExprType::UnOp) {
        this->Type = type;
        this->Expr = std::move(expr);
    }

    UnOpType Type;
    std::unique_ptr<CastExpression> Expr;
};

class SizeOfExpression : public UnaryExpression {

    explicit SizeOfExpression(std::unique_ptr<UnaryExpression> expr) : UnaryExpression(UnExprType::SizeOf) {
        this->Expr = std::move(expr);
    }

    std::unique_ptr<UnaryExpression> Expr;
};

#endif //EPICALYX_UNARY_EXPRESSION_H
