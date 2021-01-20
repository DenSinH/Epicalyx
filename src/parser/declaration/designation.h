#ifndef EPICALYX_DESIGNATION_H
#define EPICALYX_DESIGNATION_H

#include "../AST.h"
#include <stdexcept>

class Designator : public Decl {

};

class ArrayMemberDesignator : public Designator {
public:
    explicit ArrayMemberDesignator(NODE(Expr)& member) {
        Member = std::move(member);

        if (!Member->IsConstant()) {
            throw std::runtime_error("Array member designator is not a constant");
        }
    }

    NODE(Expr) Member;  // must be constant
};

class StructFieldDesignator : public Designator {
public:
    explicit StructFieldDesignator(std::string& name) {
        Name = name;
    }

    std::string Name;
};

#endif //EPICALYX_DESIGNATION_H
