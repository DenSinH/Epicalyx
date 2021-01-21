#ifndef EPICALYX_DECLARATOR_H
#define EPICALYX_DECLARATOR_H

#include "AST.h"
#include "pointer.h"

class Declarator;

class DirectDeclaratorPostfix : public Decl {
public:

};

class AbstractDeclarator : public Decl {
public:
    AbstractDeclarator() {

    }

    void AddPostfix(NODE(DirectDeclaratorPostfix)& postfix) {
        Postfixes.push_back(std::move(postfix));
    }

    void AddPointer(NODE(Pointer)& postfix) {
        Pointers.push_back(std::move(postfix));
    }

    std::vector<NODE(Pointer)> Pointers = {};
    std::vector<NODE(DirectDeclaratorPostfix)> Postfixes = {};

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "AbstractDeclarator: " };
        for (auto& ptr : Pointers) {
            for (auto& s : ptr->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }

        for (auto& pf : Postfixes) {
            for (auto& s : pf->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }
        return repr;
    }
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

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "ArrayPostfix: " };
        std::string qualifiers;
        if (Static) {
            qualifiers += "static ";
        }

        for (auto& tq : Qualifiers) {
            qualifiers += tq->String() + " ";
        }

        if (Pointer) {
            qualifiers += "* ";
        }
        repr.push_back(qualifiers);

        return repr;
    }
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

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "ParameterDeclaration: " };

        for (auto& s : Specifiers->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }

        for (auto& s : Declar->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }

        return repr;
    }
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

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "DirectDeclaratorFunctionPostfix: " };

        for (auto& param : ParameterTypeList) {
            for (auto& s : param->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }

        if (Variadic) {
            repr.emplace_back("...");
        }
        return repr;
    }
};

class Declarator : public AbstractDeclarator {
public:
    explicit Declarator(std::string& name) : Name(name), AbstractDeclarator() {

    }

    const std::string Name;

    std::list<std::string> Repr() override {
        return { Name };
    }
};

#endif //EPICALYX_DECLARATOR_H
