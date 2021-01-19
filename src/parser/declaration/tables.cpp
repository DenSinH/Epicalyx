#include "declaration_specifier.h"

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

const std::map<enum TokenType, StorageClassSpecifier::StorageClass> StorageClassSpecifier::TokenMap = {
        { TokenType::Typedef, StorageClass::Typedef },
        { TokenType::Extern, StorageClass::Extern },
        { TokenType::Static, StorageClass::Static },
        { TokenType::ThreadLocal, StorageClass::ThreadLocal },
        { TokenType::Auto, StorageClass::Auto },
        { TokenType::Register, StorageClass::Register },
};