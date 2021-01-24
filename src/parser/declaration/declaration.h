#ifndef EPICALYX_DECLARATION_H
#define EPICALYX_DECLARATION_H

#include "AST.h"
#include "specifiers.h"
#include "initializer.h"
#include "declarator.h"

class InitDeclarator : public DeclNode {
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

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "InitDeclarator: "};
        for (auto& s : Declar->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }
        if (Init) {
            for (auto& s : Init->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }
        return repr;
    }
};

class Declaration : public DeclNode {
public:
    explicit Declaration(NODE(DeclarationSpecifiers)& specifiers) {
        Specifiers = std::move(specifiers);
    }

    void AddDeclarator(NODE(InitDeclarator)& declarator) {
        InitDeclarators.push_back(std::move(declarator));
    }

    NODE(DeclarationSpecifiers) Specifiers;
    std::vector<NODE(InitDeclarator)> InitDeclarators;

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "Declaration: "};
        for (auto& s : Specifiers->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }
        for (auto& id : InitDeclarators) {
            for (auto& s : id->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }
        return repr;
    }
};


#endif //EPICALYX_DECLARATION_H
