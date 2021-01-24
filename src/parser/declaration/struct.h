#ifndef EPICALYX_STRUCT_H
#define EPICALYX_STRUCT_H

#include "AST.h"
#include "specifiers.h"
#include "declarator.h"
#include <stdexcept>

class StructDeclarator : public DeclNode {
public:
    explicit StructDeclarator(NODE(AbstractDeclarator)& field, NODE(ExprNode)& size) {
        this->Field = std::move(field);
        this->Size = std::move(size);

        if (Size && !Size->IsConstant()) {
            throw std::runtime_error("Explicit struct declarator size is not a constant expression");
        }
    }

    explicit StructDeclarator(NODE(ExprNode)& size) {
        this->Size = std::move(size);

        if (Size && !Size->IsConstant()) {
            throw std::runtime_error("Explicit struct declarator size is not a constant expression");
        }
    }

    explicit StructDeclarator(NODE(AbstractDeclarator)& field) {
        this->Field = std::move(field);
    }

    NODE(AbstractDeclarator) Field = nullptr;  // declarator, opt
    NODE(ExprNode) Size = nullptr;  // constant-expression, opt

    std::list<std::string> Repr() const override {
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


class StructDeclaration : public DeclNode {
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

    std::list<std::string> Repr() const override {
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
    explicit StructUnionSpecifier(const std::string& id, TypeSpecifierType type) :
        TypeSpecifier(type),
        ID(id) {

    }

    void AddDeclaration(NODE(StructDeclaration)& declaration) {
        DeclarationList.push_back(std::move(declaration));
    }

    const std::string ID;
    std::vector<NODE(StructDeclaration)> DeclarationList = {};

    std::list<std::string> Repr() const override {
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
    virtual std::string Keyword() const {
        return "";
    }
};

class StructSpecifier : public StructUnionSpecifier {
public:
    explicit StructSpecifier(const std::string& id) : StructUnionSpecifier(id, TypeSpecifierType::Struct) {

    }

    explicit StructSpecifier() : StructUnionSpecifier("", TypeSpecifierType::Struct) {

    }

    static bool Is(enum TokenType type) {
        return type == TokenType::Struct;
    }

protected:
    std::string Keyword() const override {
        return "struct";
    }
};

class UnionSpecifier : public StructUnionSpecifier {
public:
    explicit UnionSpecifier(const std::string& id) : StructUnionSpecifier(id, TypeSpecifierType::Union) {

    }

    explicit UnionSpecifier() : StructUnionSpecifier("", TypeSpecifierType::Union) {

    }

    static bool Is(enum TokenType type) {
        return type == TokenType::Union;
    }

protected:
    std::string Keyword() const override {
        return "union";
    }
};

#endif //EPICALYX_STRUCT_H
