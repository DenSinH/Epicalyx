#ifndef EPICALYX_INITIALIZER_H
#define EPICALYX_INITIALIZER_H

#include "../AST.h"
#include "designation.h"

class Initializer : public Decl {

};

class InitializerList : public Initializer {
public:
    explicit InitializerList(std::vector<std::pair<NODE(Designator), NODE(Initializer)>>& list) {
        List = std::move(list);
    }

    std::vector<std::pair<NODE(Designator), NODE(Initializer)>> List;
};

class AssignmentInitializer : public Initializer {
public:
    explicit AssignmentInitializer(NODE(Expr)& expression) {
        Expression = std::move(expression);
    }

    NODE(Expr) Expression;
};

#endif //EPICALYX_INITIALIZER_H
