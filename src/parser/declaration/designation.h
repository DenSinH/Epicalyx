#ifndef EPICALYX_DESIGNATION_H
#define EPICALYX_DESIGNATION_H

#include "../AST.h"
#include <stdexcept>

class Designator : public DeclNode {

};

class ArrayMemberDesignator : public Designator {
public:
    explicit ArrayMemberDesignator(NODE(ExprNode)& member) {
        Member = std::move(member);

        if (!Member->IsConstant()) {
            throw std::runtime_error("Array member designator is not a constant");
        }
    }

    NODE(ExprNode) Member;  // must be constant

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "ArrayMemberDesignator: [" };
        for (auto& s : Member->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }
        repr.emplace_back("]");
        return repr;
    }
};

class StructFieldDesignator : public Designator {
public:
    explicit StructFieldDesignator(std::string& name) {
        Name = name;
    }

    std::string Name;

    std::list<std::string> Repr() override {
        return { "." + Name };
    }
};

#endif //EPICALYX_DESIGNATION_H
