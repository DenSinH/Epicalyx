#ifndef EPICALYX_SPECIFIERS_H
#define EPICALYX_SPECIFIERS_H

#include "AST.h"
#include "../../tokenizer/tokens.h"

#include <stdexcept>
#include <map>

class TypeName;

class StorageClassSpecifier : public Decl {
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

    StorageClass Class;

    static StorageClass TokenTypeToStorageClass(enum TokenType type) {
        if (TokenMap.contains(type)) {
            return TokenMap.at(type);
        }
        throw std::runtime_error("Invalid storage class specifier: " + Token::TypeString(type));
    }
private:
    static const std::map<enum TokenType, StorageClass> TokenMap;
};

class TypeSpecifier : public Decl {
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

    TypeSpecifierType Type;

    static TypeSpecifierType TokenTypeToTypeSpecifierType(enum TokenType type) {
        if (TokenMap.contains(type)) {
            return TokenMap.at(type);
        }
        throw std::runtime_error("Invalid type specifier: " + Token::TypeString(type));
    }
    
private:
    static const std::map<enum TokenType, TypeSpecifierType> TokenMap;
};

class AtomicTypeSpecifier : public TypeSpecifier {
public:
    AtomicTypeSpecifier(NODE(TypeName)& type_name) : TypeSpecifier(TypeSpecifierType::AtomicType) {
        this->Type = std::move(type_name);
    }

    NODE(TypeName) Type;  // type-name
};

class TypedefName : public TypeSpecifier {
public:
    explicit TypedefName(std::string& name) : TypeSpecifier(TypeSpecifierType::TypedefName) {
        this->Name = name;
    }

    std::string Name;
};

class TypeQualifier : public Decl {
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

    explicit TypeQualifier(TokenType qualifier) {
        switch(qualifier) {
            case TokenType::Const:
                Qualifier = TypeQualifierType::Const;
                break;
            case TokenType::Restrict:
                Qualifier = TypeQualifierType::Restrict;
                break;
            case TokenType::Volatile:
                Qualifier = TypeQualifierType::Volatile;
                break;
            case TokenType::Atomic:
                Qualifier = TypeQualifierType::Atomic;
                break;
            default:
                throw std::runtime_error("Invalid type qualifier: " + Token::TypeString(qualifier));
        }
    }

    TypeQualifierType Qualifier;
};

class FunctionSpecifier : public Decl {
public:
    enum class FunctionSpecifierType {
        Inline,
        Noreturn,
    };

    explicit FunctionSpecifier(TokenType type) {
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

    FunctionSpecifierType Type;
};

class AlignmentSpecifier : public Decl {

};

class AlignmentSpecifierExpr : public AlignmentSpecifier {
public:

    explicit AlignmentSpecifierExpr(NODE(Expr)& expression) {
        Expression = std::move(expression);
        if (!Expression->IsConstant()) {
            throw std::runtime_error("Alignment specifier expression is not a constant");
        }
    }

    NODE(Expr) Expression;
};

class AlignmentSpecifierTypeName : public AlignmentSpecifier {
public:

    explicit AlignmentSpecifierTypeName(NODE(Decl)& type_name) {
        Type = std::move(type_name);
    }

    NODE(Decl) Type;
};

class DeclarationSpecifiers : public Decl {
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
};

#endif //EPICALYX_SPECIFIERS_H
