#ifndef EPICALYX_INITIALIZER_H
#define EPICALYX_INITIALIZER_H

#include "AST.h"
#include "designation.h"

class Initializer : public Node {
public:
    explicit Initializer(const TOKEN& tok) : Node(tok) {

    }
};

class InitializerList : public Initializer {
public:
    explicit InitializerList(const TOKEN& tok) : Initializer(tok) {

    }

    explicit InitializerList(const TOKEN& tok, std::vector<std::pair<std::vector<NODE(Designator)>, NODE(Initializer)>>&& list) : Initializer(tok) {
        List = std::move(list);
    }

    void AddInitializer(std::vector<NODE(Designator)>&& designators, NODE(Initializer)&& initializer) {
        List.emplace_back(std::move(designators), std::move(initializer) );
    }

    std::vector<std::pair<std::vector<NODE(Designator)>, NODE(Initializer)>> List = {};

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "InitializerList: " };
        for (auto& di : List) {
            NestedRepr(repr, di.first);
            NestedRepr(repr, di.second);
        }
        return repr;
    }

    // todo: in CType: check initializer lists
    /*
     * std::vector<std::pair<std::vector<std::variant<std::string, CTYPE>>, CTYPE>>
     * empty vector: anonymous member
     * otherwise: string for struct / union field
     *            CTYPE for array element
     * */
};

class AssignmentInitializer : public Initializer {
public:
    explicit AssignmentInitializer(const TOKEN& tok, NODE(ExprNode)&& expression) : Initializer(tok) {
        Expression = std::move(expression);
    }

    NODE(ExprNode) Expression;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "AssignmentInitializer: " };
        NestedRepr(repr, Expression);
        return repr;
    }
};

#endif //EPICALYX_INITIALIZER_H
