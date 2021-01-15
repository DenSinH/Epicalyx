#ifndef EPICALYX_CAST_EXPRESSION_H
#define EPICALYX_CAST_EXPRESSION_H


#include <memory>
#include <utility>
#include "../AST.h"
#include "binop_expression.h"

// CondExpr can also just be a CastExpression
class CastExpression : public CondExpr {
public:
    CastExpression() : CondExpr() {
        this->TypeName = "";
        this->Expr = nullptr;
    }

    CastExpression(std::string& type_name, std::unique_ptr<CastExpression> expr) : CondExpr() {
        this->TypeName = type_name;
        this->Expr = std::move(expr);
    }

    std::string TypeName;
    std::unique_ptr<CastExpression> Expr;
};

#endif //EPICALYX_CAST_EXPRESSION_H
