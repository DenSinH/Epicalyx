#ifndef EPICALYX_SELECTION_H
#define EPICALYX_SELECTION_H

#include "AST.h"
#include "log.h"


class IfStatement : public StatementNode {
public:
    explicit IfStatement(const TOKEN& tok, NODE(ExprNode)&& condition, NODE(StatementNode)&& _if, NODE(StatementNode)&& _else) :
            StatementNode(tok),
            Condition(std::move(condition)),
            If(std::move(_if)),
            Else(std::move(_else)) {

    }

    explicit IfStatement(const TOKEN& tok, NODE(ExprNode)&& condition, NODE(StatementNode)&& _if) :
            StatementNode(tok),
            Condition(std::move(condition)),
            If(std::move(_if)),
            Else(nullptr) {

    }

    const NODE(ExprNode) Condition;
    const NODE(StatementNode) If;
    const NODE(StatementNode) Else;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "if: "};
        NestedRepr(repr, Condition);
        repr.emplace_back("do:");

        NestedRepr(repr, If);
        if (Else) {
            repr.emplace_back("else:");
            NestedRepr(repr, Else);
        }

        return repr;
    }
};

class Switch : public StatementNode {
public:
    explicit Switch(const TOKEN& tok, NODE(ExprNode)&& expression, NODE(StatementNode)&& statement) :
            StatementNode(tok),
            Expression(std::move(expression)),
            Statement(std::move(statement)) {

    }

    const NODE(ExprNode) Expression;
    const NODE(StatementNode) Statement;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "switch: "};
        NestedRepr(repr, Expression);
        repr.emplace_back("statement:");

        NestedRepr(repr, Statement);
        return repr;
    }
};

#endif //EPICALYX_SELECTION_H
