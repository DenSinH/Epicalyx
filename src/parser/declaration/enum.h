#ifndef EPICALYX_ENUM_H
#define EPICALYX_ENUM_H

#include "../AST.h"
#include <stdexcept>
#include "specifiers.h"

class Enumerator : public Decl {
public:
    explicit Enumerator(std::string& name, NODE(Expr)& value) {
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
    NODE(Expr) Value = nullptr;  // constant-expression / nullptr for default value
};

class EnumSpecifier : public TypeSpecifier {
public:
    explicit EnumSpecifier(std::string& id) : TypeSpecifier(TypeSpecifierType::Enum) {
        this->ID = id;
    }

    void AddEnumerator(NODE(Enumerator)& enumerator) {
        EnumeratorList.push_back(std::move(enumerator));
    }

    std::string ID = "";
    std::vector<NODE(Enumerator)> EnumeratorList = {};
};

#endif //EPICALYX_ENUM_H
