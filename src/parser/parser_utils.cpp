#include "parser.h"
#include "declaration/specifiers.h"
#include "declaration/struct.h"
#include "declaration/enum.h"
#include "declaration/typename.h"

#include <set>

static const std::set<enum TokenType> TypeNameIndicators = {
        TokenType::Const,
        TokenType::Restrict,
        TokenType::Volatile,
        TokenType::Atomic,
        TokenType::Void,
        TokenType::Char,
        TokenType::Short,
        TokenType::Int,
        TokenType::Long,
        TokenType::Float,
        TokenType::Double,
        TokenType::Signed,
        TokenType::Unsigned,
        TokenType::Bool,
        TokenType::Complex,
        TokenType::Struct,
        TokenType::Union,
        TokenType::Enum,
};

bool Parser::IsTypedefName(const TOKEN& token) {
    if (token->Class == TokenClass::Identifier) {
        if (TypedefNames.contains(std::static_pointer_cast<Identifier>(token)->Name)) {
            return true;
        }
    }
    return false;
}

bool Parser::IsTypeSpecifier(const TOKEN& token) {
    return TypeSpecifier::Is(token->Type)
           || AtomicTypeSpecifier::Is(token->Type)
           || StructSpecifier::Is(token->Type)
           || UnionSpecifier::Is(token->Type)
           || EnumSpecifier::Is(token->Type)
           || IsTypedefName(token);
}

bool Parser::IsTypeName(size_t offset) {
    /*
     * Tracing the grammar of type-name:
     *      type-name: specifier-qualifier-list (abstract-decl)opt
     *
     *      specifier-qualfier-list:
     *          | type-specifier (specifier-qualifier-list)opt
     *          | type-qualifier (specifier-qualifier-list)opt
     *
     *       type-qualifier: (only the following keywords):
     *          | const | volatile | restrict | _Atomic
     *
     *       type-specifier:
     *          some keywords (int/char/etc.)
     *          | atomic-type-specifier -> _Atomic( type-name )
     *          | struct-or-union -> struct / union ...
     *          | enum-specifier -> enum ...
     *          | typedef-name -> identifier
     *
     * So if we keep track of typedef-name's we find, we can unambiguously detect whether the next string of tokens
     * is a type name or not.
     * */

    auto start = LookAhead(offset);
    if (!start) {
        // token out of range
        return false;
    }

    if (TypeNameIndicators.contains(start->Type)) {
        return true;
    }

    if (IsTypedefName(start)) {
        return true;
    }

    return false;
}

bool Parser::IsDeclarationSpecifier(const TOKEN& token) {
    return StorageClassSpecifier::Is(token->Type)
           || IsTypeSpecifier(token)
           || TypeQualifier::Is(token->Type)
           || FunctionSpecifier::Is(token->Type)
           || AlignmentSpecifier::Is(token->Type);
}