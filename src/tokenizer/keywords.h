#ifndef EPICALYX_KEYWORDS_H
#define EPICALYX_KEYWORDS_H

#include "tokens.h"
#include <map>
#include <string>

static const std::map<const std::string, const Keyword> Keywords = {
        { "auto",         Keyword(TokenType::Auto) },
        { "break",        Keyword(TokenType::Break) },
        { "case",         Keyword(TokenType::Case) },
        { "char",         Keyword(TokenType::Char) },
        { "const",        Keyword(TokenType::Const) },
        { "continue",     Keyword(TokenType::Continue) },
        { "default",      Keyword(TokenType::Default) },
        { "do",           Keyword(TokenType::Do) },
        { "double",       Keyword(TokenType::Double) },
        { "else",         Keyword(TokenType::Else) },
        { "enum",         Keyword(TokenType::Enum) },
        { "extern",       Keyword(TokenType::Extern) },
        { "float",        Keyword(TokenType::Float) },
        { "for",          Keyword(TokenType::For) },
        { "goto",         Keyword(TokenType::Goto) },
        { "if",           Keyword(TokenType::If) },
        { "inline",       Keyword(TokenType::Inline) },
        { "int",          Keyword(TokenType::Int) },
        { "long",         Keyword(TokenType::Long) },
        { "register",     Keyword(TokenType::Register) },
        { "restrict",     Keyword(TokenType::Restrict) },
        { "return",       Keyword(TokenType::Return) },
        { "short",        Keyword(TokenType::Short) },
        { "signed",       Keyword(TokenType::Signed) },
        { "sizeof",       Keyword(TokenType::Sizeof) },
        { "static",       Keyword(TokenType::Static) },
        { "struct",       Keyword(TokenType::Struct) },
        { "switch",       Keyword(TokenType::Switch) },
        { "typedef",      Keyword(TokenType::Typedef) },
        { "union",        Keyword(TokenType::Union) },
        { "unsigned",     Keyword(TokenType::Unsigned) },
        { "void",         Keyword(TokenType::Void) },
        { "volatile",     Keyword(TokenType::Volatile) },
        { "while",        Keyword(TokenType::While) },
        { "_Alignas",      Keyword(TokenType::Alignas) },
        { "_Alignof",      Keyword(TokenType::Alignof) },
        { "_Atomic",       Keyword(TokenType::Atomic) },
        { "_Bool",         Keyword(TokenType::Bool) },
        { "_Complex",      Keyword(TokenType::Complex) },
        { "_Generic",      Keyword(TokenType::Generic) },
        { "_Imaginary",    Keyword(TokenType::Imaginary) },
        { "_Noreturn",     Keyword(TokenType::Noreturn) },
        { "_Static_assert", Keyword(TokenType::StaticAssert) },
        { "_Thread_local",  Keyword(TokenType::ThreadLocal) },
};

#endif //EPICALYX_KEYWORDS_H
