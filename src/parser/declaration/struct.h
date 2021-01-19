#ifndef EPICALYX_STRUCT_H
#define EPICALYX_STRUCT_H

#include "../AST.h"
#include <stdexcept>


class StructDeclarationList : public Decl {
public:
    explicit StructDeclarationList(NODE(Decl)& list, NODE(Decl)& head) {
        this->List = std::move(list);
        this->Head = std::move(head);
    }

    NODE(Decl) List;  // struct-declaration-list
    NODE(Decl) Head;  // struct-declaration
};

class StructDeclaration : public Decl {
public:
    explicit StructDeclaration(NODE(Decl)& specifiers_qualifiers, NODE(Decl)& declarators) {
        this->SpecifiersQualifies = std::move(specifiers_qualifiers);
        this->Declarators = std::move(declarators);
    }

    explicit StructDeclaration(NODE(Decl)& specifiers_qualifiers) {
        this->SpecifiersQualifies = std::move(specifiers_qualifiers);
    }

    NODE(Decl) SpecifiersQualifies;    // specifier-qualifier-list
    NODE(Decl) Declarators = nullptr;  // struct-declarator-list, opt
};

class StructDeclaratorList : public Decl {
public:
    explicit StructDeclaratorList(NODE(Decl)& list, NODE(Decl)& head) {
        this->List = std::move(list);
        this->Head = std::move(head);
    }

    NODE(Decl) List;  // struct-declarator-list
    NODE(Decl) Head;  // struct-declarator
};

class StructDeclarator : public Decl {
public:
    explicit StructDeclarator(NODE(Decl)& name, NODE(Expr)& size) {
        this->Name = std::move(name);
        this->Size = std::move(size);

        if (Size && !Size->IsConstant()) {
            throw std::runtime_error("Explicit struct declarator size is not a constant expression");
        }
    }

    explicit StructDeclarator(NODE(Expr)& size) {
        this->Size = std::move(size);

        if (Size && !Size->IsConstant()) {
            throw std::runtime_error("Explicit struct declarator size is not a constant expression");
        }
    }

    explicit StructDeclarator(NODE(Decl)& name) {
        this->Name = std::move(name);
    }

    NODE(Decl) Name = nullptr;  // declarator, opt
    NODE(Expr) Size = nullptr;  // constant-expression, opt
};

#endif //EPICALYX_STRUCT_H
