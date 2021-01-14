#ifndef EPICALYX_CAST_EXPRESSION_H
#define EPICALYX_CAST_EXPRESSION_H


#include <memory>
#include <utility>
#include "../AST.h"

class CastExpression : public Node {
public:
    CastExpression() {
        this->TypeName = "";
        this->Expr = nullptr;
    }

    CastExpression(std::string& type_name, std::unique_ptr<CastExpression> expr) {
        this->TypeName = type_name;
        this->Expr = std::move(expr);
    }

    std::string TypeName;
    std::unique_ptr<CastExpression> Expr;
};

#endif //EPICALYX_CAST_EXPRESSION_H
