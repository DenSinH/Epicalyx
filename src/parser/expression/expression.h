#ifndef EPICALYX_EXPRESSION_H
#define EPICALYX_EXPRESSION_H

#include "../AST.h"

class Expression : public Expr {
public:
    Expression(NODE(Expr)& left, NODE(Expr)& right) {
        this->Left = std::move(left);
        this->Right = std::move(right);
    }

    NODE(Expr) Left;
    NODE(Expr) Right;

    std::list<std::string> Repr() override {
        std::list<std::string> repr = {};
        for (auto& s : Left->Repr()) {
            repr.emplace_back(s);
        }
        for (auto& s : Right->Repr()) {
            repr.emplace_back(s);
        }
        return repr;
    }
};

#endif //EPICALYX_EXPRESSION_H
