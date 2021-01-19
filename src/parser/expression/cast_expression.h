#ifndef EPICALYX_CAST_EXPRESSION_H
#define EPICALYX_CAST_EXPRESSION_H

#include "../AST.h"

// CondExpr can also just be a CastExpression
class CastExpression : public Expr {
public:
    CastExpression(std::string& type_name, NODE(Expr)& right) {
        this->TypeName = type_name;
        this->Right = std::move(right);
    }

    std::string TypeName;
    NODE(Expr) Right;

    std::vector<std::string> Repr() override {
        std::vector<std::string> repr = { "CastExpr: (" + TypeName + ")" };
        for (auto& s : Right->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }

    bool IsConstant() override {
        return Right->IsConstant();
    }
};

#endif //EPICALYX_CAST_EXPRESSION_H
