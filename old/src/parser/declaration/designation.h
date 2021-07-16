#ifndef EPICALYX_DESIGNATION_H
#define EPICALYX_DESIGNATION_H

#include "AST.h"
#include <stdexcept>
#include <utility>

class Designator : public Node {
public:
    explicit Designator(const TOKEN& tok) : Node(tok) {

    }
};

class ArrayMemberDesignator : public Designator {
public:
    explicit ArrayMemberDesignator(const TOKEN& tok, NODE(ExprNode)&& member) : Designator(tok) {
        Member = std::move(member);

//        if (!Member->IsConstant(<#initializer#>)) {
//            throw std::runtime_error("Array member designator is not a constant");
//        }
    }

    NODE(ExprNode) Member;  // must be constant

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "ArrayMemberDesignator: [" };
        NestedRepr(repr, Member);
        repr.emplace_back("]");
        return repr;
    }
};

class StructFieldDesignator : public Designator {
public:
    explicit StructFieldDesignator(const TOKEN& tok, std::string name) :
            Designator(tok),
            Name(std::move(name)) {

    }

    const std::string Name;

    std::list<std::string> Repr() const override {
        return { "." + Name };
    }
};

#endif //EPICALYX_DESIGNATION_H
