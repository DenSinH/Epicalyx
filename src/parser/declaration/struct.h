#ifndef EPICALYX_STRUCT_H
#define EPICALYX_STRUCT_H

#include "AST.h"
#include "specifiers.h"
#include <stdexcept>

class StructDeclaration : public Decl {
public:
    explicit StructDeclaration() {
        Declarators = nullptr;
    }

    explicit StructDeclaration(NODE(Decl)& declarators) {
        this->Declarators = std::move(declarators);
    }

    template<typename T>
    void AddSpecifierQualifier(NODE(T)&);

    void  AddSpecifierQualifier(NODE(TypeSpecifier)& specifier) {
        Specifiers.push_back(std::move(specifier));
    }

    void  AddSpecifierQualifier(NODE(TypeQualifier)& qualifier) {
        Qualifiers.push_back(std::move(qualifier));
    }

    std::vector<NODE(TypeSpecifier)> Specifiers;
    std::vector<NODE(TypeQualifier)> Qualifiers;

    NODE(Decl) Declarators = nullptr;  // struct-declarator-list, opt
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


template<TypeSpecifier::TypeSpecifierType T>
class StructUnionSpecifier : public TypeSpecifier {
public:
    static_assert(T == TypeSpecifierType::Struct || T == TypeSpecifierType::Union, "Invalid struct or union specifier");

    explicit StructUnionSpecifier(std::string& id) : TypeSpecifier(T) {
        this->ID = id;
    }

    explicit StructUnionSpecifier() : StructUnionSpecifier("") {

    }

    void AddDeclaration(NODE(StructDeclaration)& declaration) {
        DeclarationList.push_back(std::move(declaration));
    }

    std::string ID = "";
    std::vector<NODE(StructDeclaration)> DeclarationList = {};
};

#endif //EPICALYX_STRUCT_H
