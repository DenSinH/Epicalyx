#ifndef EPICALYX_DECLARATION_SPECIFIER_H
#define EPICALYX_DECLARATION_SPECIFIER_H

#include "../AST.h"
#include "../../tokenizer/tokens.h"
#include <stdexcept>

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

    static constexpr StorageClass TokenTypeToStorageClass(enum TokenType type) {
        switch(type) {
            case TokenType::Typedef:
                return StorageClass::Typedef;
            case TokenType::Extern:
                return StorageClass::Extern;
            case TokenType::Static:
                return StorageClass::Static;
            case TokenType::ThreadLocal:
                return StorageClass::ThreadLocal;
            case TokenType::Auto:
                return StorageClass::Auto;
            case TokenType::Register:
                return StorageClass::Register;
            default:
                throw std::runtime_error("Invalid storage class specifier: " + Token::TypeString(type));
        }
    }
};

class TypeSpecifier : public Decl {
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
};

#endif //EPICALYX_DECLARATION_SPECIFIER_H
