#ifndef EPICALYX_EMPTY_H
#define EPICALYX_EMPTY_H

#include "AST.h"

class EmptyStatement : public StatementNode {
public:
    EmptyStatement(const TOKEN& tok) : StatementNode(tok) {

    }

    std::list<std::string> Repr() const override {
        return { "nothing" };
    }
};

#endif //EPICALYX_EMPTY_H
