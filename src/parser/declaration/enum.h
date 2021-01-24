#ifndef EPICALYX_ENUM_H
#define EPICALYX_ENUM_H

#include "../AST.h"
#include <stdexcept>
#include "specifiers.h"

class Enumerator : public DeclNode {
public:
    explicit Enumerator(std::string& name, NODE(ExprNode)& value) {
        this->Name = name;
        this->Value = std::move(value);

        if (Value && !Value->IsConstant()) {
            throw std::runtime_error("Explicit enum value is not a constant expression");
        }
    }

    explicit Enumerator(std::string& name) {
        this->Name = name;
    }

    std::string Name;
    NODE(ExprNode) Value = nullptr;  // constant-expression / nullptr for default value

    std::list<std::string> Repr() override {
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
    explicit EnumSpecifier() : TypeSpecifier(TypeSpecifierType::Enum) {

    }

    explicit EnumSpecifier(std::string& id) : TypeSpecifier(TypeSpecifierType::Enum) {
        this->ID = id;
    }

    void AddEnumerator(NODE(Enumerator)& enumerator) {
        EnumeratorList.push_back(std::move(enumerator));
    }

    std::string ID = "";
    std::vector<NODE(Enumerator)> EnumeratorList = {};

    std::list<std::string> Repr() override {
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
