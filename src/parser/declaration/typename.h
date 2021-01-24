#ifndef EPICALYX_TYPENAME_H
#define EPICALYX_TYPENAME_H

#include "AST.h"
#include "specifiers.h"
#include "declarator.h"

class TypeName : public TypeNode {
public:
    explicit TypeName() {

    }

    explicit TypeName(NODE(AbstractDeclarator)& declar) {
        Declar = std::move(declar);
    }

    void AddSpecifier(NODE(TypeSpecifier)& specifier) {
        TypeSpecifiers.push_back(std::move(specifier));
    }

    void AddQualifier(NODE(TypeQualifier)& qualifier) {
        TypeQualifiers.push_back(std::move(qualifier));
    }

    NODE(AbstractDeclarator) Declar = nullptr;  // optional
    std::vector<NODE(TypeSpecifier)> TypeSpecifiers = {};
    std::vector<NODE(TypeQualifier)> TypeQualifiers = {};

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "TypeName: "};
        for (auto& tq : TypeQualifiers) {
            for (auto& s : tq->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }

        for (auto& ts : TypeSpecifiers) {
            for (auto& s : ts->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }

        if (Declar) {
            for (auto& s : Declar->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }
        return repr;
    }
};

class AtomicTypeSpecifier : public TypeSpecifier {
public:
    explicit AtomicTypeSpecifier(NODE(TypeName)& type_name) : TypeSpecifier(TypeSpecifierType::AtomicType) {
        this->Type = std::move(type_name);
    }

    NODE(TypeName) Type;  // type-name

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "AtomicTypeSpecifier: "};
        for (auto& s : Type->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }
        return repr;
    }
};

class AlignmentSpecifierTypeName : public AlignmentSpecifier {
public:

    explicit AlignmentSpecifierTypeName(NODE(TypeName)& type_name) {
        Type = std::move(type_name);
    }

    NODE(TypeName) Type;

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "AlignmentSpecifierTypeName: "};
        for (auto& s : Type->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }
        return repr;
    }
};

#endif //EPICALYX_TYPENAME_H
