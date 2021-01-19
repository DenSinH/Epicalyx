#ifndef EPICALYX_COND_EXPRESSION_H
#define EPICALYX_COND_EXPRESSION_H

#include "../AST.h"

class CondExpr : public Expr {
public:
    CondExpr(NODE(Expr)& left,
    NODE(Expr)& t,
    NODE(Expr)& f) {
        this->Left = std::move(left);
        this->True = std::move(t);
        this->False = std::move(f);
    }

    NODE(Expr) Left;
    NODE(Expr) True;
    NODE(Expr) False;

    std::vector<std::string> Repr() override {
        std::vector<std::string> repr = { "CondExpr: Cond:" };
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

    bool IsConstant() override {
        if (!Left->IsConstant()) {
            return false;
        }

        // todo:
        // if Left->Eval() / return True->IsConstant() / False->IsConstant()
        return True->IsConstant() && False->IsConstant();
    }
};

#endif //EPICALYX_COND_EXPRESSION_H
