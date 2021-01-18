#ifndef EPICALYX_CAST_EXPRESSION_H
#define EPICALYX_CAST_EXPRESSION_H


#include <memory>
#include <utility>
#include "../AST.h"

// CondExpr can also just be a CastExpression
class CastExpression : public Expr {
public:
    CastExpression(std::string& type_name, std::shared_ptr<Expr> right) {
        this->TypeName = type_name;
        this->Right = std::move(right);
    }

    std::string TypeName;
    std::shared_ptr<Expr> Right;

    std::vector<std::string> Repr() override {
        std::vector<std::string> repr = { "CastExpr: (" + TypeName + ")" };
        for (auto& s : Right->Repr()) {
            repr.emplace_back("    " + s);
        }
        return repr;
    }
};

#endif //EPICALYX_CAST_EXPRESSION_H
