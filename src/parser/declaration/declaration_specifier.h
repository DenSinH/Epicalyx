#ifndef EPICALYX_DECLARATION_SPECIFIER_H
#define EPICALYX_DECLARATION_SPECIFIER_H

#include "../AST.h"
#include "../../tokenizer/tokens.h"
#include <stdexcept>
#include <map>

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
    AtomicTypeSpecifier(NODE(Decl)& type_name) : TypeSpecifier(TypeSpecifierType::AtomicType) {
        this->TypeName = std::move(type_name);
    }

    NODE(Decl) TypeName;
};

template<TypeSpecifier::TypeSpecifierType T>
class StructUnionSpecifier : public TypeSpecifier {
public:
    static_assert(T == TypeSpecifierType::Struct || T == TypeSpecifierType::Union, "Invalid struct or union specifier");

    explicit StructUnionSpecifier(std::string& id, NODE(Decl)& declaration_list) : TypeSpecifier(T) {
        this->ID = id;
        this->DeclarationList = std::move(declaration_list);
    }

    explicit StructUnionSpecifier(std::string& id) : TypeSpecifier(T) {
        this->ID = id;
    }

    explicit StructUnionSpecifier(NODE(Decl)& declaration_list) : StructUnionSpecifier("", declaration_list) {

    }

    std::string ID = "";
    NODE(Decl) DeclarationList = nullptr;
};


class EnumSpecifier : public TypeSpecifier {
public:
    explicit EnumSpecifier(std::string& id, NODE(Decl)& declaration_list) : TypeSpecifier(TypeSpecifierType::Enum) {
        this->ID = id;
        this->DeclarationList = std::move(declaration_list);
    }

    explicit EnumSpecifier(std::string& id) : TypeSpecifier(TypeSpecifierType::Enum) {
        this->ID = id;
    }

    explicit EnumSpecifier(NODE(Decl)& declaration_list) : TypeSpecifier(TypeSpecifierType::Enum) {
        this->DeclarationList = std::move(declaration_list);
    }

    std::string ID = "";
    NODE(Decl) DeclarationList = nullptr;
};

class TypedefName : public TypeSpecifier {
public:
    explicit TypedefName(std::string& name) : TypeSpecifier(TypeSpecifierType::TypedefName) {
        this->Name = name;
    }


    std::string Name;
};

#endif //EPICALYX_DECLARATION_SPECIFIER_H
