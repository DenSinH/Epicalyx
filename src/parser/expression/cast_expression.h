#ifndef EPICALYX_CAST_EXPRESSION_H
#define EPICALYX_CAST_EXPRESSION_H


#include <memory>
#include <utility>
#include "../AST.h"

// CondExpr can also just be a CastExpression
class CastExpression : public Expr {
public:
    CastExpression(std::string& type_name, std::unique_ptr<Expr> expr) {
        this->TypeName = type_name;
        this->Expr = std::move(expr);
    }

    std::string TypeName;
    std::unique_ptr<Expr> Expr;
};

#endif //EPICALYX_CAST_EXPRESSION_H
