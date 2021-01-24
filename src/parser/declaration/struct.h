#ifndef EPICALYX_STRUCT_H
#define EPICALYX_STRUCT_H

#include "AST.h"
#include "specifiers.h"
#include "declarator.h"
#include <stdexcept>

class StructDeclarator : public Decl {
public:
    explicit StructDeclarator(NODE(AbstractDeclarator)& field, NODE(Expr)& size) {
        this->Field = std::move(field);
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

    explicit StructDeclarator(NODE(AbstractDeclarator)& field) {
        this->Field = std::move(field);
    }

    NODE(AbstractDeclarator) Field = nullptr;  // declarator, opt
    NODE(Expr) Size = nullptr;  // constant-expression, opt

    std::list<std::string> Repr() override {
        // std::list<std::string> repr = { "StructDeclarator:" };
        std::list<std::string> repr = { };

        if (Field) {
            repr.emplace_back("Declarator:");
            for (auto& s : Field->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }
        if (Size) {
            repr.emplace_back("Size:");
            for (auto& s : Size->Repr()) {
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

class StructUnionSpecifier : public TypeSpecifier {
public:
    explicit StructUnionSpecifier(TypeSpecifierType type) : TypeSpecifier(type) {

    }

    void AddDeclaration(NODE(StructDeclaration)& declaration) {
        DeclarationList.push_back(std::move(declaration));
    }

    std::string ID = "";
    std::vector<NODE(StructDeclaration)> DeclarationList = {};

    std::list<std::string> Repr() override {
        std::list<std::string> repr = {};
        repr.push_back(Keyword() + " " + ID);

        for (auto& decl : DeclarationList) {
            for (auto& s : decl->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }
        return repr;
    }

protected:
    virtual std::string Keyword() {
        return "";
    }
};

class StructSpecifier : public StructUnionSpecifier {
public:
    explicit StructSpecifier(std::string& id) : StructUnionSpecifier(TypeSpecifierType::Struct) {
        this->ID = id;
    }

    explicit StructSpecifier() : StructUnionSpecifier(TypeSpecifierType::Struct) {

    }

    static bool Is(enum TokenType type) {
        return type == TokenType::Struct;
    }

protected:
    std::string Keyword() override {
        return "struct";
    }
};

class UnionSpecifier : public StructUnionSpecifier {
public:
    explicit UnionSpecifier(std::string& id) : StructUnionSpecifier(TypeSpecifierType::Union) {
        this->ID = id;
    }

    explicit UnionSpecifier() : StructUnionSpecifier(TypeSpecifierType::Union) {

    }

    static bool Is(enum TokenType type) {
        return type == TokenType::Union;
    }

protected:
    std::string Keyword() override {
        return "union";
    }
};

#endif //EPICALYX_STRUCT_H
