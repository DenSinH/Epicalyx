#ifndef EPICALYX_EXPRESSION_H
#define EPICALYX_EXPRESSION_H

#include "../AST.h"

class Expression : public ExprNode {
public:
    Expression(NODE(ExprNode)& left, NODE(ExprNode)& right) {
        this->Left = std::move(left);
        this->Right = std::move(right);
    }

    NODE(ExprNode) Left;
    NODE(ExprNode) Right;

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
