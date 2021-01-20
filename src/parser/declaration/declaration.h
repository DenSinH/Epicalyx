#ifndef EPICALYX_DECLARATION_H
#define EPICALYX_DECLARATION_H

#include "AST.h"
#include "specifiers.h"
#include "initializer.h"

class Declarator;

class InitDeclarator : public Decl {
public:
    explicit InitDeclarator(NODE(Declarator)& declarator) {
        Declar = std::move(declarator);
        Init = nullptr;
    }

    explicit InitDeclarator(NODE(Declarator)& declarator, NODE(Initializer)& initializer) {
        Declar = std::move(declarator);
        Init = std::move(initializer);
    }

    NODE(Declarator) Declar;
    NODE(Initializer) Init;
};

class Declaration : public Decl {
public:
    explicit Declaration(NODE(DeclarationSpecifiers)& specifiers) {
        Specifiers = std::move(specifiers);
    }

    void AddDeclarator(NODE(InitDeclarator)& declarator) {
        InitDeclarators.push_back(std::move(declarator));
    }

    NODE(DeclarationSpecifiers) Specifiers;
    std::vector<NODE(InitDeclarator)> InitDeclarators;
};


#endif //EPICALYX_DECLARATION_H
