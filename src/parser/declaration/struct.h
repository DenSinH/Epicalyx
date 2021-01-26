#ifndef EPICALYX_STRUCT_H
#define EPICALYX_STRUCT_H

#include "AST.h"
#include "specifiers.h"
#include "declarator.h"
#include <stdexcept>

class StructDeclarator : public Node {
public:
    explicit StructDeclarator(const TOKEN& tok, NODE(Declarator)& field, NODE(ExprNode)& size) : Node(tok) {
        this->Field = std::move(field);
        this->Size = std::move(size);

        if (Size && !Size->IsConstant()) {
            throw std::runtime_error("Explicit struct declarator size is not a constant expression");
        }
    }

    explicit StructDeclarator(const TOKEN& tok, NODE(ExprNode)& size) : Node(tok) {
        this->Size = std::move(size);

        if (Size && !Size->IsConstant()) {
            throw std::runtime_error("Explicit struct declarator size is not a constant expression");
        }
    }

    explicit StructDeclarator(const TOKEN& tok, NODE(Declarator)& field) : Node(tok) {
        this->Field = std::move(field);
    }

    NODE(Declarator) Field = nullptr;  // declarator, opt
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


class StructDeclaration : public Node {
public:
    explicit StructDeclaration(const TOKEN& tok) : Node(tok){

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
    explicit StructUnionSpecifier(const TOKEN& tok, const std::string& id, TypeSpecifierType type) :
        TypeSpecifier(tok, type),
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

    std::string String() const override {
        return Keyword() + " " + ID + " { " + std::to_string(DeclarationList.size()) + " declarations }";
    }

protected:
    virtual std::string Keyword() const {
        return "";
    }
};

class StructSpecifier : public StructUnionSpecifier {
public:
    explicit StructSpecifier(const TOKEN& tok, const std::string& id) : StructUnionSpecifier(tok, id, TypeSpecifierType::Struct) {

    }

    explicit StructSpecifier(const TOKEN& tok) : StructUnionSpecifier(tok, "", TypeSpecifierType::Struct) {

    }

    static bool Is(enum TokenType type) {
        return type == TokenType::Struct;
    }

    std::string Keyword() const override {
        return "struct";
    }
};

class UnionSpecifier : public StructUnionSpecifier {
public:
    explicit UnionSpecifier(const TOKEN& tok, const std::string& id) : StructUnionSpecifier(tok, id, TypeSpecifierType::Union) {

    }

    explicit UnionSpecifier(const TOKEN& tok) : StructUnionSpecifier(tok, "", TypeSpecifierType::Union) {

    }

    static bool Is(enum TokenType type) {
        return type == TokenType::Union;
    }

    std::string Keyword() const override {
        return "union";
    }
};

#endif //EPICALYX_STRUCT_H
