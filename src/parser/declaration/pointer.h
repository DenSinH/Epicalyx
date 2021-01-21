#ifndef EPICALYX_POINTER_H
#define EPICALYX_POINTER_H

#include "AST.h"
#include "specifiers.h"

class Pointer : public SpecifierQualifier {
public:
    Pointer() = default;

    void AddQualifier(NODE(TypeQualifier)& qualifier) {
        QualifierList.push_back(std::move(qualifier));
    }

    std::vector<NODE(TypeQualifier)> QualifierList;

    std::string String() override {
        std::string qualifiers;

        for (auto& q : QualifierList) {
            qualifiers += q->String() + " ";
        }
        return qualifiers + "*";
    }
};

#endif //EPICALYX_POINTER_H
