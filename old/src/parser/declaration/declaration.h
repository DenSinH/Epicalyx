#ifndef EPICALYX_DECLARATION_H
#define EPICALYX_DECLARATION_H

#include "AST.h"
#include "specifiers.h"
#include "initializer.h"
#include "declarator.h"

class InitDeclarator : public DeclNode {
public:
    explicit InitDeclarator(const TOKEN& tok, NODE(Declarator)&& declarator) : DeclNode(tok) {
        Declar = std::move(declarator);
        Init = nullptr;
    }

    explicit InitDeclarator(const TOKEN& tok, NODE(Declarator)&& declarator, NODE(Initializer)&& initializer) : DeclNode(tok) {
        Declar = std::move(declarator);
        Init = std::move(initializer);
    }

    NODE(Declarator) Declar;
    NODE(Initializer) Init = nullptr;  // optional

    std::string GetName() const {
        return Declar->GetName();
    }

    std::list<std::string> Repr() const override {
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
    explicit Declaration(const TOKEN& tok, NODE(DeclarationSpecifiers)&& specifiers) : DeclNode(tok) {
        Specifiers = std::move(specifiers);
    }

    void AddDeclarator(NODE(InitDeclarator)& declarator) {
        InitDeclarators.push_back(std::move(declarator));
    }

    NODE(DeclarationSpecifiers) Specifiers;
    std::vector<NODE(InitDeclarator)> InitDeclarators;


    virtual void AddTypedefNames(std::set<std::string>& typedef_names) {
        // keep track of typedef names
        if (!Specifiers->IsTypedef()) {
            return;
        }

        for (auto& decl : InitDeclarators) {
            typedef_names.insert(decl->GetName());
        }
    }

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "Declaration: "};

        NestedRepr(repr, Specifiers);
        NestedRepr(repr, InitDeclarators);

        return repr;
    }
};

#endif //EPICALYX_DECLARATION_H