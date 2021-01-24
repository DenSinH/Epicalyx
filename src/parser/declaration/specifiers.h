#ifndef EPICALYX_SPECIFIERS_H
#define EPICALYX_SPECIFIERS_H

#include "AST.h"
#include "../../tokenizer/tokens.h"

#include <stdexcept>
#include <map>

class TypeName;

class StorageClassSpecifier : public SpecifierQualifier {
public:
    enum class StorageClass {
        Typedef,
        Extern,
        Static,
        ThreadLocal,
        Auto,
        Register,
    };

    explicit StorageClassSpecifier(StorageClass cls) {
        this->Class = cls;
    };

    explicit StorageClassSpecifier(enum TokenType type) : StorageClassSpecifier(TokenTypeToStorageClass(type)) {

    };

    StorageClass Class;

    static StorageClass TokenTypeToStorageClass(enum TokenType type) {
        if (TokenMap.contains(type)) {
            return TokenMap.at(type);
        }
        throw std::runtime_error("Invalid storage class specifier: " + Token::TypeString(type));
    }

    std::string String() override {
        return StringMap.at(Class);
    }

    static bool Is(enum TokenType type) {
        return TokenMap.contains(type);
    }

private:
    static const std::map<enum TokenType, StorageClass> TokenMap;
    static const std::map<StorageClassSpecifier::StorageClass, const std::string> StringMap;
};

class TypeSpecifier : public SpecifierQualifier {
public:
    enum class TypeSpecifierType {
        Void,
        Char,
        Short,
        Int,
        Long,
        Float,
        Double,
        Signed,
        Unsigned,
        Bool,
        Complex,

        AtomicType,
        Struct,
        Union,
        Enum,
        TypedefName,
    };

    explicit TypeSpecifier(TypeSpecifierType type) {
        this->Type = type;
    }

    explicit TypeSpecifier(enum TokenType type) : TypeSpecifier(TokenTypeToTypeSpecifierType(type)) {

    }

    TypeSpecifierType Type;

    static TypeSpecifierType TokenTypeToTypeSpecifierType(enum TokenType type) {
        if (TokenMap.contains(type)) {
            return TokenMap.at(type);
        }
        throw std::runtime_error("Invalid type specifier: " + Token::TypeString(type));
    }

    std::string String() override {
        return StringMap.at(Type);
    }

    static bool Is(enum TokenType type) {
        if (TokenMap.contains(type)) {
            return true;
        }
        return false;
    }

private:
    static const std::map<enum TokenType, TypeSpecifierType> TokenMap;
    static const std::map<TypeSpecifier::TypeSpecifierType, const std::string> StringMap;
};

class TypedefName : public TypeSpecifier {
public:
    explicit TypedefName(const std::string& name) :
        TypeSpecifier(TypeSpecifierType::TypedefName),
        Name(name) {
        
    }

    const std::string Name;

    std::string String() override {
        return "TypedefName(" + Name + ")";
    }
};

class TypeQualifier : public SpecifierQualifier {
public:
    enum class TypeQualifierType {
        Const,
        Restrict,
        Volatile,
        Atomic,
    };

    explicit TypeQualifier(TypeQualifierType qualifier) {
        Qualifier = qualifier;
    }

    explicit TypeQualifier(enum TokenType qualifier) {
        if (TokenMap.contains(qualifier)) {
            Qualifier = TokenMap.at(qualifier);
        }
        else {
            throw std::runtime_error("Invalid type qualifier: " + Token::TypeString(qualifier));
        }
    }

    static bool Is(enum TokenType type) {
        return TokenMap.contains(type);
    }

    std::string String() override {
        static const std::map<TypeQualifierType, std::string> StringMap = {
                { TypeQualifierType::Const, "const" },
                { TypeQualifierType::Restrict, "restrict" },
                { TypeQualifierType::Volatile, "volatile" },
                { TypeQualifierType::Atomic, "_Atomic" },
        };
        return StringMap.at(Qualifier);
    }

    static const std::map<enum TokenType, TypeQualifierType> TokenMap;

    TypeQualifierType Qualifier;
};

class FunctionSpecifier : public SpecifierQualifier {
public:
    enum class FunctionSpecifierType {
        Inline,
        Noreturn,
    };

    explicit FunctionSpecifier(enum TokenType type) {
        if (type == TokenType::Inline) {
            Type = FunctionSpecifierType::Inline;
        }
        else if (type == TokenType::Noreturn) {
            Type = FunctionSpecifierType::Noreturn;
        }
        else {
            throw std::runtime_error("Invalid function specifier type: " + Token::TypeString(type));
        }
    }

    explicit FunctionSpecifier(FunctionSpecifierType type) {
        Type = type;
    }

    std::string String() override {
        return Type == FunctionSpecifierType::Inline ? "inline" : "_Noreturn";
    }

    static bool Is(enum TokenType type) {
        return (type == TokenType::Inline) || (type == TokenType::Noreturn);
    }

    FunctionSpecifierType Type;
};

class AlignmentSpecifier : public SpecifierQualifier {
public:
    static bool Is(enum TokenType type) {
        return type == TokenType::Alignas;
    }
};

class AlignmentSpecifierExpr : public AlignmentSpecifier {
public:

    explicit AlignmentSpecifierExpr(NODE(ExprNode)& expression) {
        Expression = std::move(expression);
        if (!Expression->IsConstant()) {
            throw std::runtime_error("Alignment specifier expression is not a constant");
        }
    }

    NODE(ExprNode) Expression;

    std::string String() override {
        return "_AlignOf";
    }

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "AlignmentSpecifier (expression):" };
        for (auto& s : Expression->Repr()) {
            repr.push_back(REPR_PADDING + s);
        }
        return repr;
    }
};

class DeclarationSpecifiers : public DeclNode {
public:
    DeclarationSpecifiers() = default;

    template<typename T>
    void AddSpecifier(NODE(T)& specifier);

    template<>
    void AddSpecifier(NODE(StorageClassSpecifier)& specifier) {
        StorageClassSpecifiers.push_back(std::move(specifier));
    }
    template<>
    void AddSpecifier(NODE(TypeSpecifier)& specifier) {
        TypeSpecifiers.push_back(std::move(specifier));
    }
    template<>
    void AddSpecifier(NODE(TypeQualifier)& specifier) {
        TypeQualifiers.push_back(std::move(specifier));
    }
    template<>
    void AddSpecifier(NODE(FunctionSpecifier)& specifier) {
        FunctionSpecifiers.push_back(std::move(specifier));
    }
    template<>
    void AddSpecifier(NODE(AlignmentSpecifier)& specifier) {
        AlignmentSpecifiers.push_back(std::move(specifier));
    }

    std::vector<NODE(StorageClassSpecifier)> StorageClassSpecifiers = {};
    std::vector<NODE(TypeSpecifier)> TypeSpecifiers = {};
    std::vector<NODE(TypeQualifier)> TypeQualifiers = {};
    std::vector<NODE(FunctionSpecifier)> FunctionSpecifiers = {};
    std::vector<NODE(AlignmentSpecifier)> AlignmentSpecifiers = {};

    bool Empty() const noexcept {
        return StorageClassSpecifiers.empty() && TypeSpecifiers.empty() && TypeQualifiers.empty() && FunctionSpecifiers.empty() && AlignmentSpecifiers.empty();
    }

    std::list<std::string> Repr() override {
        std::list<std::string> repr = { "DeclarationSpecifiers: " };
        std::string specifiers_qualifiers;
        for (auto& sc : StorageClassSpecifiers) {
            specifiers_qualifiers += sc->String() + " ";
//            for (auto& s : sc->Repr()) {
//                repr.push_back(REPR_PADDING + s);
//            }
        }
        for (auto& ts : TypeSpecifiers) {
            specifiers_qualifiers += ts->String() + " ";
//            for (auto& s : ts->Repr()) {
//                repr.push_back(REPR_PADDING + s);
//            }
        }
        for (auto& tq : TypeQualifiers) {
            specifiers_qualifiers += tq->String() + " ";
//            for (auto& s : tq->Repr()) {
//                repr.push_back(REPR_PADDING + s);
//            }
        }
        for (auto& fs : FunctionSpecifiers) {
            specifiers_qualifiers += fs->String() + " ";
//            for (auto& s : fs->Repr()) {
//                repr.push_back(REPR_PADDING + s);
//            }
        }
        for (auto& as : AlignmentSpecifiers) {
            specifiers_qualifiers += as->String() + " ";
//            for (auto& s : as->Repr()) {
//                repr.push_back(REPR_PADDING + s);
//            }
        }
        repr.push_back(REPR_PADDING + specifiers_qualifiers);
        return repr;
    }
};

#endif //EPICALYX_SPECIFIERS_H
