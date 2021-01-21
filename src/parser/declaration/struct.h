#ifndef EPICALYX_STRUCT_H
#define EPICALYX_STRUCT_H

#include "AST.h"
#include "specifiers.h"
#include "declarator.h"
#include <stdexcept>

class StructDeclarator : public Decl {
public:
    explicit StructDeclarator(NODE(Declarator)& name, NODE(Expr)& size) {
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

    explicit StructDeclarator(NODE(Declarator)& name) {
        this->Name = std::move(name);
    }

    NODE(Declarator) Name = nullptr;  // declarator, opt
    NODE(Expr) Size = nullptr;  // constant-expression, opt

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "StructDeclarator:" };

        if (Name) {
            repr.emplace_back("Declarator:");
            for (auto& s : Name->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }
        if (Size) {
            repr.emplace_back("Size:");
            for (auto& s : Name->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }

        return repr;
    }
};


class StructDeclaration : public Decl {
public:
    explicit StructDeclaration() {

    }

    void AddSpecifier(NODE(TypeSpecifier)& specifier) {
        Specifiers.push_back(std::move(specifier));
    }

    void AddQualifier(NODE(TypeQualifier)& qualifier) {
        Qualifiers.push_back(std::move(qualifier));
    }

    void AddDeclarator(NODE(StructDeclarator)& declarator) {
        Declarators.push_back(std::move(declarator));
    }

    std::vector<NODE(TypeQualifier)> Qualifiers;
    std::vector<NODE(TypeSpecifier)> Specifiers;
    std::vector<NODE(StructDeclarator)> Declarators;

    std::list<std::string> Repr() override {
        std::list<std::string> repr = {};
        std::string qualifiers;
        for (auto& q : Qualifiers) {
            qualifiers += q->String() + " ";
        }

        for (auto& s : Specifiers) {
            qualifiers += s->String() + " ";
        }
        repr.push_back(qualifiers);

        for (auto& decl : Declarators) {
            for (auto& s : decl->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }
        return repr;
    }
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

    std::list<std::string> Repr() override {
        std::list<std::string> repr = {};
        std::string name;
        if constexpr(T == TypeSpecifierType::Struct) {
            name = "struct ";
        }
        else {
            name = "enum ";
        }
        name += ID;
        repr.push_back(name);

        for (auto& decl : DeclarationList) {
            for (auto& s : decl->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }
        return repr;
    }
};

#endif //EPICALYX_STRUCT_H
