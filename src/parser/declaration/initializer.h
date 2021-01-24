#ifndef EPICALYX_INITIALIZER_H
#define EPICALYX_INITIALIZER_H

#include "../AST.h"
#include "designation.h"

class Initializer : public DeclNode {

};

class InitializerList : public Initializer {
public:
    explicit InitializerList() {

    }

    explicit InitializerList(std::vector<std::pair<std::vector<NODE(Designator)>, NODE(Initializer)>>& list) {
        List = std::move(list);
    }

    void AddInitializer(std::vector<NODE(Designator)>& designators, NODE(Initializer)& initializer) {
        List.emplace_back(std::move(designators), std::move(initializer) );
    }

    std::vector<std::pair<std::vector<NODE(Designator)>, NODE(Initializer)>> List = {};

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "InitializerList: " };
        for (auto& di : List) {
            for (auto& des : di.first) {
                for (auto& s : des->Repr()) {
                    repr.push_back(REPR_PADDING + s);
                }
            }
            for (auto& s : di.second->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }
        return repr;
    }
};

class AssignmentInitializer : public Initializer {
public:
    explicit AssignmentInitializer(NODE(ExprNode)& expression) {
        Expression = std::move(expression);
    }

    NODE(ExprNode) Expression;

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "AssignmentInitializer: " };
        for (auto& s : Expression->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }
        return repr;
    }
};

#endif //EPICALYX_INITIALIZER_H
