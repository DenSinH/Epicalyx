#ifndef EPICALYX_TYPENAME_H
#define EPICALYX_TYPENAME_H

#include "AST.h"
#include "specifiers.h"
#include "declarator.h"

class TypeName : public Decl {
public:
    TypeName() {

    }

    TypeName(NODE(AbstractDeclarator)& declar) {
        Declar = std::move(declar);
    }

    void AddSpecifier(NODE(TypeSpecifier)& specifier) {
        TypeSpecifiers.push_back(std::move(specifier));
    }

    void AddQualifier(NODE(TypeQualifier)& qualifier) {
        TypeQualifiers.push_back(std::move(qualifier));
    }

    NODE(AbstractDeclarator) Declar;
    std::vector<NODE(TypeSpecifier)> TypeSpecifiers = {};
    std::vector<NODE(TypeQualifier)> TypeQualifiers = {};
};

#endif //EPICALYX_TYPENAME_H
