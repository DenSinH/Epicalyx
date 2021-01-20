#ifndef EPICALYX_POINTER_H
#define EPICALYX_POINTER_H

#include "AST.h"
#include "specifiers.h"

class Pointer : public Decl {
public:
    Pointer() = default;

    void AddQualifier(NODE(TypeQualifier)& qualifier) {
        QualifierList.push_back(std::move(qualifier));
    }

    std::vector<NODE(TypeQualifier)> QualifierList;
};

#endif //EPICALYX_POINTER_H
