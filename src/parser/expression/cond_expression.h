#ifndef EPICALYX_COND_EXPRESSION_H
#define EPICALYX_COND_EXPRESSION_H

#include "../AST.h"

class CondExpression : public ExprNode {
public:
    CondExpression(NODE(ExprNode)& left,
                   NODE(ExprNode)& t,
                   NODE(ExprNode)& f) {
        this->Left = std::move(left);
        this->True = std::move(t);
        this->False = std::move(f);
    }

    NODE(ExprNode) Left;
    NODE(ExprNode) True;
    NODE(ExprNode) False;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "CondExpr: Cond:" };
        for (auto& s : Left->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        repr.emplace_back("True:");
        for (auto& s : True->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        repr.emplace_back("False:");
        for (auto& s : False->Repr()) {
            repr.emplace_back(REPR_PADDING + s);
        }
        return repr;
    }

    bool IsConstant() const override {
        if (!Left->IsConstant()) {
            return false;
        }

        // todo:
        // if Left->Eval() / return True->IsConstant() / False->IsConstant()
        return True->IsConstant() && False->IsConstant();
    }
};


#endif //EPICALYX_COND_EXPRESSION_H
