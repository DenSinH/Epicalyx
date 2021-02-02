#ifndef EPICALYX_COMPOUND_H
#define EPICALYX_COMPOUND_H

#include "AST.h"

class CompoundStatement : public StatementNode {
public:
    explicit CompoundStatement(const TOKEN& tok) :
            StatementNode(tok) {

    }

    void AddStatement(NODE(BlockItem)&& statement) {
        Statements.emplace_back(std::move(statement));
    }

    std::vector<NODE(BlockItem)> Statements = {};

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "Compound statement: {" };
        NestedRepr(repr, Statements);
        repr.emplace_back("}");
        return repr;
    }
};

#endif //EPICALYX_COMPOUND_H
