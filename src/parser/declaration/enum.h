#ifndef EPICALYX_ENUM_H
#define EPICALYX_ENUM_H

#include "../AST.h"
#include <stdexcept>

class EnumeratorList : public Decl {
public:
    explicit EnumeratorList(NODE(Decl)& list, NODE(Decl)& value) {
        this->List = std::move(list);
        this->Value = std::move(value);
    }

    NODE(Decl) List;   // enumerator-list
    NODE(Decl) Value;  // enumerator
};

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

#endif //EPICALYX_ENUM_H
