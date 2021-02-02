#ifndef EPICALYX_EXPRESSION_H
#define EPICALYX_EXPRESSION_H

#include "../AST.h"

class Expression : public ExprNode {
public:
    Expression(const TOKEN& tok, NODE(ExprNode)&& left, NODE(ExprNode)&& right) :
            ExprNode(tok),
            Left(std::move(left)),
            Right(std::move(right)) {

    }

    const NODE(ExprNode) Left;
    const NODE(ExprNode) Right;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = {};
        NestedRepr(repr, Left);
        NestedRepr(repr, Right);
        return repr;
    }
};

#endif //EPICALYX_EXPRESSION_H
