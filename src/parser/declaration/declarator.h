#ifndef EPICALYX_DECLARATOR_H
#define EPICALYX_DECLARATOR_H

#include "AST.h"
#include "pointer.h"

class Declarator;
class AbstractDeclarator;

class DirectDeclaratorPostfix : public Decl {
public:

};

class DirectDeclaratorArrayPostfix : public DirectDeclaratorPostfix {
public:
    explicit DirectDeclaratorArrayPostfix(NODE(Expr)& size, bool _Static = false, bool _Pointer = false) {
        Size = std::move(size);
        Static = _Static;
        Pointer = _Pointer;
    }

    explicit DirectDeclaratorArrayPostfix(bool _Static = false, bool _Pointer = false) {
        Size = nullptr;
        Static = _Static;
        Pointer = _Pointer;
    }

    void AddQualifier(NODE(TypeQualifier)& qualifier) {
        Qualifiers.push_back(std::move(qualifier));
    }

    NODE(Expr) Size;
    bool Static = false;
    bool Pointer = false;
    std::vector<NODE(TypeQualifier)> Qualifiers = {};
};

class ParameterDeclaration : public Decl {
public:
    ParameterDeclaration(NODE(DeclarationSpecifiers)& specifiers, NODE(AbstractDeclarator)& declar) {
        Specifiers = std::move(specifiers);
        Declar = std::move(declar);
    }

    ParameterDeclaration(NODE(DeclarationSpecifiers)& specifiers) {
        // in case declar is an abstract-declarator, it is optional
        Specifiers = std::move(specifiers);
        Declar = nullptr;
    }

    NODE(DeclarationSpecifiers) Specifiers;
    NODE(AbstractDeclarator) Declar;
};

class DirectDeclaratorFunctionPostfix : public DirectDeclaratorPostfix {
public:
    explicit DirectDeclaratorFunctionPostfix(bool variadic = false) {
        Variadic = variadic;
    }

    void AddParameter(NODE(ParameterDeclaration)& parameter) {
        ParameterTypeList.push_back(std::move(parameter));
    }

    bool Variadic = false;
    std::vector<NODE(ParameterDeclaration)> ParameterTypeList = {};
};


class AbstractDeclarator : public Decl {
public:
    AbstractDeclarator() {

    }

    void AddPostfix(NODE(DirectDeclaratorPostfix)& postfix) {
        Postfixes.push_back(std::move(postfix));
    }

    std::vector<NODE(DirectDeclaratorPostfix)> Postfixes = {};
    std::vector<NODE(Pointer)> Pointers = {};
};


class Declarator : public AbstractDeclarator {
public:
    explicit Declarator(std::string& name) : Name(name), AbstractDeclarator() {

    }

    const std::string Name;
};

#endif //EPICALYX_DECLARATOR_H
