#ifndef EPICALYX_DECLARATOR_H
#define EPICALYX_DECLARATOR_H

#include "AST.h"
#include "pointer.h"

class Declarator;

class DirectDeclaratorPostfix : public Node {
public:
    explicit DirectDeclaratorPostfix(const TOKEN& tok) : Node(tok) {

    }

    virtual constexpr bool IsFunctionPostfix() const {
        return false;
    }
};

class AnyDeclarator : public Node {
public:
    explicit AnyDeclarator(const TOKEN& tok, std::vector<NODE(Pointer)>&& pointers) : Node(tok) {
        Pointers = std::move(pointers);
    }

    void AddPostfix(NODE(DirectDeclaratorPostfix)&& postfix) {
        // declarator is a function if the last postfix is a function postfix
        _IsFunction = postfix->IsFunctionPostfix();
        Postfixes.push_back(std::move(postfix));
    }

    void AddPointer(NODE(Pointer)&& postfix) {
        Pointers.push_back(std::move(postfix));
    }

    bool IsFunction() const {
        return _IsFunction;
    }

    virtual bool IsAbstract() const {
        log_fatal("Uninherited AnyDeclarator");
    }

    virtual bool HasNested() const {
        log_fatal("Uninherited AnyDeclarator");
    }

    virtual void SetNested(NODE(AnyDeclarator)&&) {
        log_fatal("Uninherited AnyDeclarator");
    }

    std::vector<NODE(Pointer)> Pointers = {};
    std::vector<NODE(DirectDeclaratorPostfix)> Postfixes = {};

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { NameRepr() };

        Node::NestedRepr(repr, Pointers);
        for (auto& s : NestedRepr()) {
            repr.push_back(REPR_PADDING + s);
        }
        Node::NestedRepr(repr, Postfixes);

        return repr;
    }

protected:
    virtual std::string NameRepr() const {
        return "";
    }

    virtual std::list<std::string> NestedRepr() const {
        return {};
    }

private:
    bool _IsFunction = false;
};

class DirectDeclaratorArrayPostfix : public DirectDeclaratorPostfix {
public:
    explicit DirectDeclaratorArrayPostfix(const TOKEN& tok, NODE(ExprNode)&& size, bool _Static = false, bool _Pointer = false) :
            DirectDeclaratorPostfix(tok),
            Size(std::move(size)),
            Static(_Static),
            Pointer(_Pointer) {

    }

    explicit DirectDeclaratorArrayPostfix(const TOKEN& tok, bool _Static = false, bool _Pointer = false) :
            DirectDeclaratorPostfix(tok),
            Size(nullptr),
            Static(_Static),
            Pointer(_Pointer) {

    }

    void AddQualifier(NODE(TypeQualifier)&& qualifier) {
        Qualifiers.push_back(std::move(qualifier));
    }

    const NODE(ExprNode) Size;
    const bool Static = false;
    const bool Pointer = false;
    std::vector<NODE(TypeQualifier)> Qualifiers = {};

    std::list<std::string> Repr() const override {
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
            NestedRepr(repr, Size);
        }

        return repr;
    }
};

class ParameterDeclaration : public Node {
public:
    ParameterDeclaration(const TOKEN& tok, NODE(DeclarationSpecifiers)&& specifiers, NODE(AnyDeclarator)&& declar) :
            Node(tok) {
        Specifiers = std::move(specifiers);
        Declar = std::move(declar);
    }

    ParameterDeclaration(const TOKEN& tok, NODE(DeclarationSpecifiers)&& specifiers) :
            Node(tok) {
        // in case declar is an abstract-declarator, it is optional
        Specifiers = std::move(specifiers);
        Declar = nullptr;
    }

    NODE(DeclarationSpecifiers) Specifiers;
    NODE(AnyDeclarator) Declar;

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "ParameterDeclaration: " };

        NestedRepr(repr, Specifiers);
        if (Declar) {
            NestedRepr(repr, Declar);
        }

        return repr;
    }
};


class DirectDeclaratorParameterListPostfix : public DirectDeclaratorPostfix {
public:
    explicit DirectDeclaratorParameterListPostfix(const TOKEN& tok, bool variadic = false) :
            DirectDeclaratorPostfix(tok) {
        Variadic = variadic;
    }

    void AddParameter(NODE(ParameterDeclaration)&& parameter) {
        ParameterTypeList.push_back(std::move(parameter));
    }

    bool Variadic = false;
    std::vector<NODE(ParameterDeclaration)> ParameterTypeList = {};

    constexpr bool IsFunctionPostfix() const override {
        return true;
    }

    std::list<std::string> Repr() const override {
        std::list<std::string> repr = { "DirectDeclaratorParameterListPostfix: " };

        NestedRepr(repr, ParameterTypeList);

        if (Variadic) {
            repr.emplace_back("...");
        }
        return repr;
    }
};


// for K&R style functions, not supported:
//class DirectDeclaratorIdentifierListPostfix : public DirectDeclaratorPostfix {
//public:
//    explicit DirectDeclaratorIdentifierListPostfix(const TOKEN& tok) : DirectDeclaratorPostfix(tok) {
//
//    }
//
//    void AddIdentifier(const std::string& identifier) {
//        IdentifierList.push_back(identifier);
//    }
//
//    std::vector<std::string> IdentifierList = {};
//
//    std::list<std::string> Repr() const override {
//        std::list<std::string> repr = { "DirectDeclaratorIdentifierListPostfix: " };
//
//        for (auto& s : IdentifierList) {
//            repr.push_back(REPR_PADDING + s);
//        }
//        return repr;
//    }
//};


class AbstractDeclarator : public AnyDeclarator {
public:
    explicit AbstractDeclarator(const TOKEN& tok, std::vector<NODE(Pointer)>&& pointers) : AnyDeclarator(tok, std::move(pointers)) {

    }

    bool HasNested() const override {
        return Nested != nullptr;
    }

    void SetNested(NODE(AnyDeclarator)&& nested) override {
        if (Nested) {
            log_fatal("Invalid handling of AbstractDeclarator::SetNested: nested already exists");
        }
        Nested = std::move(nested);
    }

    bool IsAbstract() const override {
        if (Nested) {
            return Nested->IsAbstract();
        }
        return true;
    }

    NODE(AnyDeclarator) Nested = nullptr;

protected:
    std::string NameRepr() const override {
        return "AbstractDeclarator: ";
    }

    std::list<std::string> NestedRepr() const override {
        std::list<std::string> repr = {};
        if (Nested) {
            repr.emplace_back("Nested:");
            Node::NestedRepr(repr, Nested);
        }
        return repr;
    }
};


class Declarator : public AnyDeclarator {
public:
    explicit Declarator(const TOKEN& tok, NODE(Declarator)&& nested, std::vector<NODE(Pointer)>&& pointers) :
            AnyDeclarator(tok, std::move(pointers)),
            Name(""),
            Nested(std::move(nested)) {

    }

    explicit Declarator(const TOKEN& tok, const std::string& name, std::vector<NODE(Pointer)>&& pointers) :
            AnyDeclarator(tok, std::move(pointers)),
            Name(name),
            Nested(nullptr) {

    }

    std::string GetName() const {
        if (Nested) {
            return Nested->GetName();
        }
        return Name;
    }

    const std::string Name;
    const NODE(Declarator) Nested;

    bool IsAbstract() const override {
        if (Nested) {
            return Nested->IsAbstract();
        }
        return false;
    }

protected:
    std::string NameRepr() const override {
        return "Declarator: " + Name;
    }

    std::list<std::string> NestedRepr() const override {
        std::list<std::string> repr = {};
        if (Nested) {
            repr.emplace_back("Nested:");
            Node::NestedRepr(repr, Nested);
        }
        return repr;
    }
};

#endif //EPICALYX_DECLARATOR_H
