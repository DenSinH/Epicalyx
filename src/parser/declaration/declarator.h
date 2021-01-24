#ifndef EPICALYX_DECLARATOR_H
#define EPICALYX_DECLARATOR_H

#include "AST.h"
#include "pointer.h"

class Declarator;

class DirectDeclaratorPostfix : public Node {
public:

};

class AbstractDeclarator : public DeclNode {
public:
    AbstractDeclarator() {

    }

    void SetNested(NODE(AbstractDeclarator)& nested) {
        if (Nested) {
            log_fatal("Invalid handling of AbstractDeclarator::SetNested: nested already exists");
        }
        Nested = std::move(nested);
    }

    void AddPostfix(NODE(DirectDeclaratorPostfix)& postfix) {
        Postfixes.push_back(std::move(postfix));
    }

    void AddPointer(NODE(Pointer)& postfix) {
        Pointers.push_back(std::move(postfix));
    }

    virtual bool IsAbstract() {
        if (Nested) {
            return Nested->IsAbstract();
        }
        return true;
    }

    std::vector<NODE(Pointer)> Pointers = {};
    NODE(AbstractDeclarator) Nested = nullptr;
    std::vector<NODE(DirectDeclaratorPostfix)> Postfixes = {};

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "AbstractDeclarator: " };
        for (auto& ptr : Pointers) {
            for (auto& s : ptr->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }

        if (Nested) {
            repr.emplace_back("Nested:");
            for (auto& s : Nested->Repr()) {
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
    explicit DirectDeclaratorArrayPostfix(NODE(ExprNode)& size, bool _Static = false, bool _Pointer = false) {
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

    NODE(ExprNode) Size;
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

        if (Size) {
            for (auto& s : Size->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }

        return repr;
    }
};

class ParameterDeclaration : public DeclNode {
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

        if (Declar) {
            for (auto& s : Declar->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }

        return repr;
    }
};

class DirectDeclaratorParameterListPostfix : public DirectDeclaratorPostfix {
public:
    explicit DirectDeclaratorParameterListPostfix(bool variadic = false) {
        Variadic = variadic;
    }

    void AddParameter(NODE(ParameterDeclaration)& parameter) {
        ParameterTypeList.push_back(std::move(parameter));
    }

    bool Variadic = false;
    std::vector<NODE(ParameterDeclaration)> ParameterTypeList = {};

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "DirectDeclaratorParameterListPostfix: " };

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

class DirectDeclaratorIdentifierListPostfix : public DirectDeclaratorPostfix {
public:
    explicit DirectDeclaratorIdentifierListPostfix() {

    }

    void AddIdentifier(const std::string& identifier) {
        IdentifierList.push_back(identifier);
    }

    std::vector<std::string> IdentifierList = {};

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "DirectDeclaratorIdentifierListPostfix: " };

        for (auto& s : IdentifierList) {
            repr.push_back(REPR_PADDING + s);
        }
        return repr;
    }
};

class Declarator : public AbstractDeclarator {
public:
    explicit Declarator(std::string& name) : Name(name), AbstractDeclarator() {

    }

    explicit Declarator(std::string& name, std::vector<NODE(Pointer)>& pointers) : Declarator(name) {
        Pointers = std::move(pointers);
    }

    const std::string Name;

    bool IsAbstract() override {
        if (Nested) {
            return Nested->IsAbstract();
        }
        return false;
    }

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "Declarator: " };
        for (auto& ptr : Pointers) {
            for (auto& s : ptr->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }

        repr.push_back("ID: " + Name);

        for (auto& pf : Postfixes) {
            for (auto& s : pf->Repr()) {
                repr.push_back(REPR_PADDING + s);
            }
        }
        return repr;
    }
};

#endif //EPICALYX_DECLARATOR_H
