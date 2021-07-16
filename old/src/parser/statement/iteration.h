#ifndef EPICALYX_ITERATION_H
#define EPICALYX_ITERATION_H

#include "AST.h"

class While : public StatementNode {
public:
    explicit While(const TOKEN& tok, NODE(ExprNode)&& condition, NODE(StatementNode)&& body) :
            StatementNode(tok),
            Condition(std::move(condition)),
            Body(std::move(body)) {

    }

    const NODE(ExprNode) Condition;
    const NODE(StatementNode) Body;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "while: {" };
        NestedRepr(repr, Condition);
        repr.emplace_back("do:");

        NestedRepr(repr, Body);
        return repr;
    }
};

class DoWhile : public StatementNode {
public:
    explicit DoWhile(const TOKEN& tok, NODE(StatementNode)&& body, NODE(ExprNode)&& condition) :
            StatementNode(tok),
            Body(std::move(body)),
            Condition(std::move(condition)) {

    }

    const NODE(StatementNode) Body;
    const NODE(ExprNode) Condition;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "do: {" };
        NestedRepr(repr, Body);

        repr.emplace_back("while:");
        NestedRepr(repr, Condition);

        return repr;
    }
};

class For : public StatementNode {
public:
    explicit For(
                const TOKEN& tok,
                NODE(DeclNode)&& declaration,
                NODE(ExprNode)&& initialization,
                NODE(ExprNode)&& condition,
                NODE(ExprNode)&& updation,
                NODE(StatementNode)&& body) :
            StatementNode(tok),
            Declaration(std::move(declaration)),
            Initialization(std::move(initialization)),
            Condition(std::move(condition)),
            Updation(std::move(updation)),
            Body(std::move(body))
            {

    }

    const NODE(DeclNode) Declaration;      // optional
    const NODE(ExprNode) Initialization;   // optional
    const NODE(ExprNode) Condition;        // optional
    const NODE(ExprNode) Updation;         // optional
    const NODE(StatementNode) Body;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "for: {" };

        repr.emplace_back("declaration:");
        if (Declaration) {
            NestedRepr(repr, Declaration);
        }

        repr.emplace_back("initialization:");
        if (Initialization) {
            NestedRepr(repr, Initialization);
        }

        repr.emplace_back("condition:");
        if (Condition) {
            NestedRepr(repr, Condition);
        }

        repr.emplace_back("updation:");
        if (Updation) {
            NestedRepr(repr, Updation);
        }

        repr.emplace_back("do:");
        NestedRepr(repr, Body);

        return repr;
    }
};

#endif //EPICALYX_ITERATION_H
