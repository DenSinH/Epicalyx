#ifndef EPICALYX_ENUM_H
#define EPICALYX_ENUM_H

#include "../AST.h"
#include <stdexcept>
#include "specifiers.h"

class Enumerator : public DeclNode {
public:
    explicit Enumerator(const TOKEN& tok, std::string& name, NODE(ExprNode)& value) :
            DeclNode(tok),
            Name(name) {
        this->Value = std::move(value);

        if (Value && !Value->IsConstant()) {
            throw std::runtime_error("Explicit enum value is not a constant expression");
        }
    }

    explicit Enumerator(const TOKEN& tok, std::string& name) :
            DeclNode(tok),
            Name(name ){
    }

    const std::string Name;
    NODE(ExprNode) Value = nullptr;  // constant-expression / nullptr for default value

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "Enumerator: " + Name };
        if (Value) {
            for (auto& s : Value->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }
        return repr;
    }
};

class EnumSpecifier : public TypeSpecifier {
public:
    explicit EnumSpecifier(const TOKEN& tok) : TypeSpecifier(tok, TypeSpecifierType::Enum) {

    }

    explicit EnumSpecifier(const TOKEN& tok, const std::string& id) :
        TypeSpecifier(tok, TypeSpecifierType::Enum),
        ID(id) {

    }

    void AddEnumerator(NODE(Enumerator)& enumerator) {
        EnumeratorList.push_back(std::move(enumerator));
    }

    const std::string ID;
    std::vector<NODE(Enumerator)> EnumeratorList = {};

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "EnumSpecifier: " + ID };
        for (auto& enumerator : EnumeratorList) {
            for (auto& s : enumerator->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }
        return repr;
    }

    static bool Is(enum TokenType type) {
        return type == TokenType::Enum;
    }
};

#endif //EPICALYX_ENUM_H
