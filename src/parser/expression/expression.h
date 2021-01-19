#ifndef EPICALYX_EXPRESSION_H
#define EPICALYX_EXPRESSION_H

#include <memory>
#include <utility>
#include "../AST.h"

class Expression : public Expr {
public:
    Expression(NODE(Expr)& left, NODE(Expr)& right) {
        this->Left = std::move(left);
        this->Right = std::move(right);
    }

    NODE(Expr) Left;
    NODE(Expr) Right;

    std::vector<std::string> Repr() override {
        std::vector<std::string> repr = {};
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
