#include "specifiers.h"

const std::map<enum TokenType, TypeSpecifier::TypeSpecifierType> TypeSpecifier::TokenMap = {
        { TokenType::Void, TypeSpecifierType::Void },
        { TokenType::Char, TypeSpecifierType::Char },
        { TokenType::Short, TypeSpecifierType::Short },
        { TokenType::Int, TypeSpecifierType::Int },
        { TokenType::Long, TypeSpecifierType::Long },
        { TokenType::Float, TypeSpecifierType::Float },
        { TokenType::Double, TypeSpecifierType::Double },
        { TokenType::Signed, TypeSpecifierType::Signed },
        { TokenType::Unsigned, TypeSpecifierType::Unsigned },
        { TokenType::Bool, TypeSpecifierType::Bool },
        { TokenType::Complex, TypeSpecifierType::Complex },
};

const std::map<TypeSpecifier::TypeSpecifierType, const std::string> TypeSpecifier::StringMap = {
        { TypeSpecifierType::Void, "void" },
        { TypeSpecifierType::Char, "char" },
        { TypeSpecifierType::Short, "short" },
        { TypeSpecifierType::Int, "int" },
        { TypeSpecifierType::Long, "long" },
        { TypeSpecifierType::Float, "float" },
        { TypeSpecifierType::Double, "double" },
        { TypeSpecifierType::Signed, "signed" },
        { TypeSpecifierType::Unsigned, "unsigned" },
        { TypeSpecifierType::Bool, "_Bool" },
        { TypeSpecifierType::Complex, "_Complex" },
};

const std::map<enum TokenType, StorageClassSpecifier::StorageClass> StorageClassSpecifier::TokenMap = {
        { TokenType::Typedef, StorageClass::Typedef },
        { TokenType::Extern, StorageClass::Extern },
        { TokenType::Static, StorageClass::Static },
        { TokenType::ThreadLocal, StorageClass::ThreadLocal },
        { TokenType::Auto, StorageClass::Auto },
        { TokenType::Register, StorageClass::Register },
};

const std::map<StorageClassSpecifier::StorageClass, const std::string> StorageClassSpecifier::StringMap = {
        { StorageClass::Typedef, "typedef" },
        { StorageClass::Extern, "extern" },
        { StorageClass::Static, "static" },
        { StorageClass::ThreadLocal, "_Thread_local" },
        { StorageClass::Auto, "auto" },
        { StorageClass::Register, "register" },
};