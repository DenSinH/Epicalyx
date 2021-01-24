#ifndef EPICALYX_CAST_EXPRESSION_H
#define EPICALYX_CAST_EXPRESSION_H

#include "../AST.h"
#include "../declaration/typename.h"

// CondExpr can also just be a CastExpression
class CastExpression : public ExprNode {
public:
    CastExpression(NODE(TypeName)& type, NODE(ExprNode)& right) {
        this->Type = std::move(type);
        this->Right = std::move(right);
    }

    NODE(TypeName) Type;
    NODE(ExprNode) Right;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "CastExpr:" };
        for (auto& s : Type->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }

        repr.push_back("Value:");
        for (auto& s : Right->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }

    bool IsConstant() const override {
        return Right->IsConstant();
    }
};

#endif //EPICALYX_CAST_EXPRESSION_H
