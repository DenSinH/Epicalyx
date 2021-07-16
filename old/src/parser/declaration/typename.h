#ifndef EPICALYX_TYPENAME_H
#define EPICALYX_TYPENAME_H

#include "AST.h"
#include "specifiers.h"
#include "declarator.h"

class TypeName : public TypeSpecifierNode {
public:
    explicit TypeName(const TOKEN& tok) : TypeSpecifierNode(tok) {

    }

    explicit TypeName(const TOKEN& tok, NODE(Declarator)&& declar) : TypeSpecifierNode(tok) {
        Declar = std::move(declar);
    }

    void AddSpecifier(NODE(TypeSpecifier)&& specifier) {
        TypeSpecifiers.push_back(std::move(specifier));
    }

    void AddQualifier(NODE(TypeQualifier)&& qualifier) {
        TypeQualifiers.push_back(std::move(qualifier));
    }

    void SetDeclar(NODE(AnyDeclarator)&& declar) {
        if (Declar) {
            log_fatal("Tried to set type-name declartor when one was already present");
        }

        Declar = std::move(declar);
    }

    NODE(AnyDeclarator) Declar = nullptr;  // optional
    std::vector<NODE(TypeSpecifier)> TypeSpecifiers = {};
    std::vector<NODE(TypeQualifier)> TypeQualifiers = {};

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "TypeName: "};
        NestedRepr(repr, TypeQualifiers);
        NestedRepr(repr, TypeSpecifiers);

        if (Declar) {
            NestedRepr(repr, Declar);
        }
        return repr;
    }
};

class AtomicTypeSpecifier : public TypeSpecifier {
public:
    explicit AtomicTypeSpecifier(const TOKEN& tok, NODE(TypeName)&& type_name) : TypeSpecifier(tok, TypeSpecifierType::AtomicType) {
        this->Type = std::move(type_name);
    }

    NODE(TypeName) Type;  // type-name

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "AtomicTypeSpecifier: "};
        NestedRepr(repr, Type);
        return repr;
    }
};

class AlignmentSpecifierTypeName : public AlignmentSpecifier {
public:

    explicit AlignmentSpecifierTypeName(const TOKEN& tok, NODE(TypeName)&& type_name) : AlignmentSpecifier(tok) {
        Type = std::move(type_name);
    }

    NODE(TypeName) Type;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "AlignmentSpecifierTypeName: "};
        NestedRepr(repr, Type);
        return repr;
    }
};

#endif //EPICALYX_TYPENAME_H
